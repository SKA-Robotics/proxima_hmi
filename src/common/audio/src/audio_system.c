#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "audio_buffer.h"
#include "audio_system.h"
#include "common_result.h"

struct audio_system {
    QueueHandle_t audio_empty_buffers_queue;
    QueueHandle_t audio_full_buffers_queue;
};

static bool s_initialized = false;
static struct audio_system s_system;

static const size_t BUFFERS_COUNT = 3;
static const size_t BUFFER_SIZE = 2000; // 125ms of playback

static audio_result_t audio_system_init_buffer_pool();
static audio_result_t audio_system_teardown_queue(QueueHandle_t queue);
static audio_result_t audio_system_teardown_queues();

audio_result_t audio_system_init() {
    if (s_initialized) {
        return AUDIO_RESULT_SUCCESS;
    }

    memset(&s_system, 0, sizeof(s_system));

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

    s_initialized = true;

    return AUDIO_RESULT_SUCCESS;
}

audio_result_t audio_system_deinit() {
    if (!s_initialized) {
        return AUDIO_RESULT_SYSTEM_NOT_INITIALIZED;
    }

    RESULT_CHECK(audio_system_teardown_queues(), AUDIO_RESULT_SUCCESS);

    s_initialized = false;

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