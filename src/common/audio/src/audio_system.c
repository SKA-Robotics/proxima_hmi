#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "audio_buffer.h"
#include "audio_system.h"
#include "common_result.h"

struct audio_system {
    // Queues
    QueueHandle_t audio_empty_buffers_queue;
    QueueHandle_t audio_full_buffers_queue;

    // Playback & feeder task
    TaskHandle_t playback_task;
    volatile bool playback_stop_requested;

    TaskHandle_t feeder_task;
    volatile bool feeder_stop_requested;

    // Streams
    SemaphoreHandle_t lock;
    audio_stream_t *active_stream;
    uint32_t next_playback_id;
    uint32_t active_playback_id;
};

static bool s_initialized = false;
static struct audio_system s_system;

static const size_t BUFFERS_COUNT = 3;
static const size_t BUFFER_SIZE = 2000; // 125ms of playback
static const size_t TASK_TEARDOWN_TIMEOUT = 100;

static audio_result_t audio_system_init_buffer_pool();
static audio_result_t audio_system_teardown_queue(QueueHandle_t queue);
static audio_result_t audio_system_teardown_queues();
static audio_result_t audio_system_teardown_playback();
static audio_result_t audio_system_drain_buffers();
static audio_result_t audio_system_start_feeder();
static audio_result_t audio_system_teardown_feeder();
static void audio_system_playback_task(void *param);
static void audio_system_feeder_task(void *param);

audio_result_t audio_system_init() {
    if (s_initialized) {
        return AUDIO_RESULT_SUCCESS;
    }

    memset(&s_system, 0, sizeof(s_system));

    // Queues
    s_system.audio_empty_buffers_queue = xQueueCreate(BUFFERS_COUNT, sizeof(audio_buffer_t *));
    if (s_system.audio_empty_buffers_queue == NULL) {
        return AUDIO_RESULT_OUT_OF_MEMORY;
    }

    s_system.audio_full_buffers_queue = xQueueCreate(BUFFERS_COUNT, sizeof(audio_buffer_t *));
    if (s_system.audio_full_buffers_queue == NULL) {
        audio_system_teardown_queues();
        return AUDIO_RESULT_OUT_OF_MEMORY;
    }

    RESULT_CHECK(audio_system_init_buffer_pool(), AUDIO_RESULT_SUCCESS);

    // Playback task
    s_system.playback_stop_requested = false;

    BaseType_t ok = xTaskCreate(audio_system_playback_task, "audio_playback", 512, NULL,
                                tskIDLE_PRIORITY + 2, &s_system.playback_task);

    if (ok != pdPASS) {
        audio_system_teardown_queues();
        return AUDIO_RESULT_OUT_OF_MEMORY;
    }

    // Streams
    s_system.lock = xSemaphoreCreateMutex();
    s_system.active_stream = NULL;
    s_system.next_playback_id = 1;
    s_system.active_playback_id = 0;

    if (s_system.lock == NULL) {
        return AUDIO_RESULT_OUT_OF_MEMORY;
    }

    s_initialized = true;

    return AUDIO_RESULT_SUCCESS;
}

audio_result_t audio_system_deinit() {
    if (!s_initialized) {
        return AUDIO_RESULT_SYSTEM_NOT_INITIALIZED;
    }

    RESULT_CHECK(audio_system_teardown_feeder(), AUDIO_RESULT_SUCCESS);
    RESULT_CHECK(audio_system_teardown_playback(), AUDIO_RESULT_SUCCESS);
    RESULT_CHECK(audio_system_teardown_queues(), AUDIO_RESULT_SUCCESS);

    vSemaphoreDelete(s_system.lock);

    s_initialized = false;

    return AUDIO_RESULT_SUCCESS;
}

audio_result_t audio_system_play_stream(audio_stream_t *stream, bool force,
                                        uint32_t *out_playback_id) {
    if (!s_initialized) {
        return AUDIO_RESULT_SYSTEM_NOT_INITIALIZED;
    }

    if (stream == NULL) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    if (xSemaphoreTake(s_system.lock, pdMS_TO_TICKS(10)) != pdTRUE) {
        return AUDIO_RESULT_OUT_OF_MEMORY;
    }

    if (s_system.active_stream != NULL && !force) {
        xSemaphoreGive(s_system.lock);
        return AUDIO_RESULT_BUSY;
    }

    if (s_system.active_stream != NULL && force) {
        audio_result_t r = audio_system_teardown_feeder();
        if (r != AUDIO_RESULT_SUCCESS) {
            xSemaphoreGive(s_system.lock);
            return r;
        }

        audio_stream_close(s_system.active_stream);
        s_system.active_stream = NULL;
        audio_system_drain_buffers();
    }

    s_system.active_stream = stream;
    s_system.active_playback_id = s_system.next_playback_id;
    s_system.next_playback_id++;

    if (out_playback_id != NULL) {
        *out_playback_id = s_system.active_playback_id;
    }

    audio_stream_reset(s_system.active_stream);

    audio_result_t r = audio_system_start_feeder();
    if (r != AUDIO_RESULT_SUCCESS) {
        xSemaphoreGive(s_system.lock);
        return r;
    }

    xSemaphoreGive(s_system.lock);
    return AUDIO_RESULT_SUCCESS;
}

static audio_result_t audio_system_init_buffer_pool() {
    for (size_t i = 0; i < BUFFERS_COUNT; i++) {
        audio_buffer_t *buffer = (audio_buffer_t *)malloc(sizeof(audio_buffer_t));

        if (buffer == NULL) {
            audio_system_teardown_queues();
            return AUDIO_RESULT_OUT_OF_MEMORY;
        }

        audio_result_t result = audio_buffer_create(buffer, BUFFER_SIZE);

        if (result != AUDIO_RESULT_SUCCESS) {
            free(buffer);
            audio_system_teardown_queues();
            return AUDIO_RESULT_OUT_OF_MEMORY;
        }

        if (xQueueSend(s_system.audio_empty_buffers_queue, &buffer, 0) != pdTRUE) {
            audio_buffer_destroy(buffer);
            free(buffer);
            audio_system_teardown_queues();
            return AUDIO_RESULT_OUT_OF_MEMORY;
        }
    }

    return AUDIO_RESULT_SUCCESS;
}

static audio_result_t audio_system_teardown_queue(QueueHandle_t queue) {
    audio_buffer_t *queued = NULL;

    if (queue != NULL) {
        while (xQueueReceive(queue, &queued, 0) == pdPASS) {
            if (queued != NULL) {
                audio_buffer_destroy(queued);
                free(queued);
                queued = NULL;
            }
        }

        vQueueDelete(queue);
    }

    return AUDIO_RESULT_SUCCESS;
}

static audio_result_t audio_system_teardown_queues() {
    RESULT_CHECK(audio_system_teardown_queue(s_system.audio_empty_buffers_queue),
                 AUDIO_RESULT_SUCCESS);
    s_system.audio_empty_buffers_queue = NULL;

    RESULT_CHECK(audio_system_teardown_queue(s_system.audio_full_buffers_queue),
                 AUDIO_RESULT_SUCCESS);
    s_system.audio_full_buffers_queue = NULL;

    return AUDIO_RESULT_SUCCESS;
}

static audio_result_t audio_system_teardown_playback() {
    s_system.playback_stop_requested = true;

    for (size_t i = 0; i < TASK_TEARDOWN_TIMEOUT / 10; i++) {
        if (s_system.playback_task == NULL) {
            return AUDIO_RESULT_SUCCESS;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return AUDIO_RESULT_TIMEOUT;
}

static audio_result_t audio_system_drain_buffers() {
    audio_buffer_t *buffer = NULL;

    while (xQueueReceive(s_system.audio_full_buffers_queue, &buffer, 0) == pdPASS) {
        if (buffer != NULL) {
            audio_buffer_reset(buffer);
            (void)xQueueSend(s_system.audio_empty_buffers_queue, &buffer, 0);
            buffer = NULL;
        }
    }

    return AUDIO_RESULT_SUCCESS;
}

static audio_result_t audio_system_start_feeder() {
    if (s_system.feeder_task != NULL) {
        return AUDIO_RESULT_SUCCESS;
    }

    s_system.feeder_stop_requested = false;

    BaseType_t ok = xTaskCreate(audio_system_feeder_task, "audio_feeder", 768, NULL,
                                tskIDLE_PRIORITY + 2, &s_system.feeder_task);

    if (ok != pdPASS) {
        s_system.feeder_task = NULL;
        return AUDIO_RESULT_OUT_OF_MEMORY;
    }

    return AUDIO_RESULT_SUCCESS;
}

static audio_result_t audio_system_teardown_feeder() {
    TaskHandle_t task = s_system.feeder_task;
    if (task == NULL) {
        return AUDIO_RESULT_SUCCESS;
    }

    s_system.feeder_stop_requested = true;

    for (size_t i = 0; i < TASK_TEARDOWN_TIMEOUT / 10; i++) {
        if (s_system.feeder_task == NULL) {
            return AUDIO_RESULT_SUCCESS;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return AUDIO_RESULT_TIMEOUT;
}

static void audio_system_playback_task(void *param) {
    (void)param;

    for (;;) {
        if (s_system.playback_stop_requested) {
            break;
        }

        audio_buffer_t *buffer = NULL;
        BaseType_t got =
            xQueueReceive(s_system.audio_full_buffers_queue, &buffer, pdMS_TO_TICKS(10));

        if (got != pdTRUE || buffer == NULL) {
            continue;
        }

        // @TODO: play buffer

        audio_buffer_reset(buffer);

        (void)xQueueSend(s_system.audio_empty_buffers_queue, &buffer, 0);
    }

    s_system.playback_task = NULL;
    vTaskDelete(NULL);
}

static void audio_system_feeder_task(void *param) {
    (void)param;

    for (;;) {
        if (s_system.feeder_stop_requested || s_system.active_stream == NULL) {
            break;
        }

        audio_buffer_t *buffer = NULL;

        if (xQueueReceive(s_system.audio_empty_buffers_queue, &buffer, pdMS_TO_TICKS(10)) !=
                pdTRUE ||
            buffer == NULL) {
            continue;
        }

        audio_result_t r = audio_stream_read(s_system.active_stream, buffer);
        if (r != AUDIO_RESULT_SUCCESS) {
            audio_buffer_reset(buffer);
            (void)xQueueSend(s_system.audio_empty_buffers_queue, &buffer, 0);
            break;
        }

        if (buffer->size == 0) {
            audio_buffer_reset(buffer);
            (void)xQueueSend(s_system.audio_empty_buffers_queue, &buffer, 0);
            break; // End of stream
        }

        if (xQueueSend(s_system.audio_full_buffers_queue, &buffer, portMAX_DELAY) != pdTRUE) {
            audio_buffer_reset(buffer);
            (void)xQueueSend(s_system.audio_empty_buffers_queue, &buffer, 0);
            break;
        }
    }

    s_system.feeder_task = NULL;
    vTaskDelete(NULL);
}