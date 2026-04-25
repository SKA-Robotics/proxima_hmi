#include "audio_decoder.h"
#include "common_result.h"

static inline void inplace_unpack_mono_lq(uint32_t *buffer, size_t samples_count) {
    uint8_t *input = (uint8_t *)buffer;

    for (size_t i = samples_count; i-- > 0;) {
        buffer[i] = ((uint32_t)input[i] << 24) | ((uint32_t)input[i] << 8);
    }
}

static inline void inplace_unpack_mono_hq(uint32_t *buffer, size_t samples_count) {
    uint16_t *input = (uint16_t *)buffer;

    for (size_t i = samples_count; i-- > 0;) {
        buffer[i] = ((uint32_t)input[i] << 16) | ((uint32_t)input[i]);
    }
}

static inline void inplace_unpack_stereo_lq(uint32_t *buffer, size_t samples_count) {
    uint8_t *input = (uint8_t *)buffer;

    for (size_t i = samples_count; i-- > 0;) {
        buffer[i] = ((uint32_t)input[i * 2] << 24) | ((uint32_t)input[i * 2 + 1] << 8);
    }
}

static inline void inplace_unpack_stereo_hq(uint32_t *buffer, size_t samples_count) {
    // nothing to do
}

audio_result_t audio_decoder_unpack_inplace(audio_buffer_t *buffer, audio_flags_t flags) {
    RESULT_CHECK(audio_buffer_check(buffer), AUDIO_RESULT_SUCCESS);

    if (buffer->size == 0) {
        return AUDIO_RESULT_BUFFER_EMPTY;
    }

    uint32_t *output = buffer->buffer;

    if (flags == AUDIO_FLAG_MONO_LQ) {
        inplace_unpack_mono_lq(output, buffer->size);
    } else if (flags == AUDIO_FLAG_MONO_HQ) {
        inplace_unpack_mono_hq(output, buffer->size);
    } else if (flags == AUDIO_FLAG_STEREO_LQ) {
        inplace_unpack_stereo_lq(output, buffer->size);
    } else if (flags == AUDIO_FLAG_STEREO_HQ) {
        inplace_unpack_stereo_hq(output, buffer->size);
    } else {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    return AUDIO_RESULT_SUCCESS;
}