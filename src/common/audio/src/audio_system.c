#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "audio_buffer.h"
#include "audio_system.h"

struct audio_system {
    QueueHandle_t audio_empty_buffers_queue;
    QueueHandle_t audio_full_buffers_queue;
};

static bool s_initialized = false;
static struct audio_system s_system;

audio_result_t audio_system_init() {
    if (s_initialized) {
        return AUDIO_RESULT_SUCCESS;
    }

    memset(&s_system, 0, sizeof(s_system));

    s_system.audio_empty_buffers_queue = xQueueCreate(3, sizeof(audio_buffer_t*));
    if (s_system.audio_empty_buffers_queue == NULL) {
        return AUDIO_RESULT_OUT_OF_MEMORY;
    }

    s_system.audio_full_buffers_queue = xQueueCreate(3, sizeof(audio_buffer_t*));
    if (s_system.audio_full_buffers_queue == NULL) {
        vQueueDelete(s_system.audio_empty_buffers_queue);
        s_system.audio_empty_buffers_queue = NULL;
        return AUDIO_RESULT_OUT_OF_MEMORY;
    }

    s_initialized = true;

    return AUDIO_RESULT_SUCCESS;
}

audio_result_t audio_system_deinit() {
    if (!s_initialized) {
        return AUDIO_RESULT_SYSTEM_NOT_INITIALIZED;
    }

    if (s_system.audio_empty_buffers_queue != NULL) {
        vQueueDelete(s_system.audio_empty_buffers_queue);
        s_system.audio_empty_buffers_queue = NULL;
    }

    if (s_system.audio_full_buffers_queue != NULL) {
        vQueueDelete(s_system.audio_full_buffers_queue);
        s_system.audio_full_buffers_queue = NULL;
    }

    return AUDIO_RESULT_SUCCESS;
}