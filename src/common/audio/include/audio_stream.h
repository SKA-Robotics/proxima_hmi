#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "common_result.h"
#include "audio_buffer.h"
#include "audio_result.h"

typedef struct audio_stream audio_stream_t;

typedef audio_result_t (*audio_stream_open_fn)(audio_stream_t *stream);
typedef audio_result_t (*audio_stream_close_fn)(audio_stream_t *stream);
typedef audio_result_t (*audio_stream_read_fn)(audio_stream_t *stream, audio_buffer_t *buffer);
typedef audio_result_t (*audio_stream_reset_fn)(audio_stream_t *stream);

typedef struct {
    audio_stream_open_fn open;
    audio_stream_close_fn close;
    audio_stream_read_fn read;
    audio_stream_reset_fn reset;
} audio_stream_ops_t;

struct audio_stream {
    const audio_stream_ops_t *ops;
    void *ctx;
};

static inline audio_result_t audio_stream_is_valid(audio_stream_t *stream) {
    if (stream == NULL || stream->ops == NULL) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    return AUDIO_RESULT_SUCCESS;
}

static inline audio_result_t audio_stream_open(audio_stream_t *stream) {
    RESULT_CHECK(audio_stream_is_valid(stream), AUDIO_RESULT_SUCCESS);

    if (stream->ops->open == NULL) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    return stream->ops->open(stream);
}

static inline audio_result_t audio_stream_close(audio_stream_t *stream) {
    RESULT_CHECK(audio_stream_is_valid(stream), AUDIO_RESULT_SUCCESS);

    if (stream->ops->close == NULL) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    return stream->ops->close(stream);
}

static inline audio_result_t audio_stream_read(audio_stream_t *stream, audio_buffer_t *buffer) {
    RESULT_CHECK(audio_stream_is_valid(stream), AUDIO_RESULT_SUCCESS);

    if (stream->ops->read == NULL || buffer == NULL) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    return stream->ops->read(stream, buffer);
}

static inline audio_result_t audio_stream_reset(audio_stream_t *stream) {
    RESULT_CHECK(audio_stream_is_valid(stream), AUDIO_RESULT_SUCCESS);

    if (stream->ops->reset == NULL) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    return stream->ops->reset(stream);
}

#ifdef __cplusplus
}
#endif