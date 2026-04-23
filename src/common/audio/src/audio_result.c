#include <stddef.h>

#include "audio_result.h"

static const char *audio_result_strings[] = {
    "AUDIO_RESULT_SUCCESS", "AUDIO_RESULT_INVALID_ARGUMENT", "AUDIO_RESULT_OUT_OF_MEMORY"};

static const size_t audio_result_strings_count =
    sizeof(audio_result_strings) / sizeof(audio_result_strings[0]);

char *audio_result_to_string(audio_result_t result) {
    if (result < 0 || result >= audio_result_strings_count) {
        return "UNKNOWN_AUDIO_RESULT";
    }

    return (char *)audio_result_strings[result];
}