#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_buffer.h"
#include "audio_format.h"
#include "audio_result.h"

audio_result_t audio_decoder_unpack_inplace(audio_buffer_t *buffer, audio_flags_t flags);

#ifdef __cplusplus
}
#endif