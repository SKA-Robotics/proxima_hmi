#include <stdint.h>

#include "audio_format.h"

inline size_t audio_format_raw_bytes_per_sample(audio_flags_t flags) {
    if (flags & AUDIO_FLAG_STEREO_HQ) {
        return sizeof(uint32_t);
    }

    if ((flags & AUDIO_FLAG_STEREO_LQ) || (flags & AUDIO_FLAG_MONO_HQ)) {
        return sizeof(uint16_t);
    }

    if (flags & AUDIO_FLAG_MONO_LQ) {
        return sizeof(uint8_t);
    }

    return 0;
}

size_t audio_format_max_raw_bytes(size_t max_samples, audio_flags_t flags) {
    const size_t bytes_per_sample = audio_format_raw_bytes_per_sample(flags);

    if (bytes_per_sample == 0) {
        return 0;
    }

    return max_samples * bytes_per_sample;
}