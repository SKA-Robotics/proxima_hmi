#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "audio_result.h"
#include "audio_stream.h"

audio_result_t audio_system_init();
audio_result_t audio_system_deinit();
audio_result_t audio_system_play_stream(audio_stream_t *stream, bool force,
                                        uint32_t *out_playback_id);

#ifdef __cplusplus
}
#endif