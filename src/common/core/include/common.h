#pragma once

#include <stdint.h>

#define UNWRAP_RESULT(value, expected)                                                             \
    if (value != expected) {                                                                       \
        return value;                                                                              \
    }

int32_t common_add(int32_t a, int32_t b);