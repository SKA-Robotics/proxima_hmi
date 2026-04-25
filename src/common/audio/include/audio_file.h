#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_format.h"
#include "audio_stream.h"

audio_result_t audio_file_create_stream(audio_stream_t *out_stream, const char *path,
                                        audio_flags_t flags);

#ifdef __cplusplus
}
#endif