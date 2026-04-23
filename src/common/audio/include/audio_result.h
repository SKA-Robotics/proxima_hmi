#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    // Common results
    AUDIO_RESULT_SUCCESS = 0,
    AUDIO_RESULT_INVALID_ARGUMENT = 1,
    AUDIO_RESULT_OUT_OF_MEMORY = 2,

    // Audio buffer results
    AUDIO_RESULT_BUFFER_NOT_INITIALIZED = 3,
    AUDIO_RESULT_BUFFER_EMPTY = 4,

    // Audio system results
    AUDIO_RESULT_SYSTEM_NOT_INITIALIZED = 5
} audio_result_t;

char *audio_result_to_string(audio_result_t result);

#ifdef __cplusplus
}
#endif