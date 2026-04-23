#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "audio_result.h"

typedef struct {
    uint32_t *buffer;
    size_t size;
    size_t max_size;
} audio_buffer_t;

audio_result_t audio_buffer_create(audio_buffer_t *buffer, const size_t max_size);
audio_result_t audio_buffer_destroy(audio_buffer_t *buffer);
audio_result_t audio_buffer_check(audio_buffer_t *buffer);
audio_result_t audio_buffer_reset(audio_buffer_t *buffer);

#ifdef __cplusplus
}
#endif