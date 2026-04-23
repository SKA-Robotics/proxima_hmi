#include <stdlib.h>

#include "common_result.h"
#include "audio_buffer.h"

audio_result_t audio_buffer_create(audio_buffer_t *buffer, const size_t max_size) {
    if (buffer == NULL || max_size == 0) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    if (max_size > SIZE_MAX / sizeof(uint32_t)) {
        return AUDIO_RESULT_OUT_OF_MEMORY;
    }

    buffer->buffer = (uint32_t *)malloc(max_size * sizeof(uint32_t));
    buffer->size = 0;
    buffer->max_size = max_size;

    if (buffer->buffer == NULL) {
        return AUDIO_RESULT_OUT_OF_MEMORY;
    }

    return AUDIO_RESULT_SUCCESS;
}

audio_result_t audio_buffer_destroy(audio_buffer_t *buffer) {
    RESULT_CHECK(audio_buffer_check(buffer), AUDIO_RESULT_SUCCESS);

    free(buffer->buffer);
    buffer->buffer = NULL;
    buffer->size = 0;
    buffer->max_size = 0;

    return AUDIO_RESULT_SUCCESS;
}

audio_result_t audio_buffer_check(audio_buffer_t *buffer) {
    if (buffer == NULL) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    if (buffer->buffer == NULL || buffer->max_size == 0) {
        return AUDIO_RESULT_BUFFER_NOT_INITIALIZED;
    }

    return AUDIO_RESULT_SUCCESS;
}

audio_result_t audio_buffer_reset(audio_buffer_t *buffer) {
    RESULT_CHECK(audio_buffer_check(buffer), AUDIO_RESULT_SUCCESS);

    buffer->size = 0;

    return AUDIO_RESULT_SUCCESS;
}