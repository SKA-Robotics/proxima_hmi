#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_result.h"

audio_result_t audio_system_init();
audio_result_t audio_system_deinit();

#ifdef __cplusplus
}
#endif