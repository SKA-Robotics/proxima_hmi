#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef enum {
    // Channels flags
    AUDIO_FLAG_MONO = 1,
    AUDIO_FLAG_STEREO = 2,

    // Quality flags
    AUDIO_FLAG_LQ = 4,
    AUDIO_FLAG_HQ = 8
} audio_flags_t;

#define AUDIO_FLAG_MONO_LQ ((audio_flags_t)(AUDIO_FLAG_MONO | AUDIO_FLAG_LQ))
#define AUDIO_FLAG_MONO_HQ ((audio_flags_t)(AUDIO_FLAG_MONO | AUDIO_FLAG_HQ))
#define AUDIO_FLAG_STEREO_LQ ((audio_flags_t)(AUDIO_FLAG_STEREO | AUDIO_FLAG_LQ))
#define AUDIO_FLAG_STEREO_HQ ((audio_flags_t)(AUDIO_FLAG_STEREO | AUDIO_FLAG_HQ))

size_t audio_format_raw_bytes_per_sample(audio_flags_t flags);
size_t audio_format_max_raw_bytes(size_t max_samples, audio_flags_t flags);

#ifdef __cplusplus
}
#endif