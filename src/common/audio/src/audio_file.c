#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"

#include "audio_decoder.h"
#include "audio_file.h"

typedef struct {
    FIL file;
    bool is_open;
    char path[128];
    audio_flags_t flags;
} audio_file_stream_ctx_t;

static audio_result_t audio_file_stream_open(audio_stream_t *stream);
static audio_result_t audio_file_stream_close(audio_stream_t *stream);
static audio_result_t audio_file_stream_read(audio_stream_t *stream, audio_buffer_t *buffer);
static audio_result_t audio_file_stream_reset(audio_stream_t *stream);

static const audio_stream_ops_t s_audio_file_ops = {
    .open = audio_file_stream_open,
    .close = audio_file_stream_close,
    .read = audio_file_stream_read,
    .reset = audio_file_stream_reset,
};

audio_result_t audio_file_create_stream(audio_stream_t *out_stream, const char *path,
                                        audio_flags_t flags) {
    if (out_stream == NULL || path == NULL) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    audio_file_stream_ctx_t *ctx = malloc(sizeof(audio_file_stream_ctx_t));
    if (ctx == NULL) {
        return AUDIO_RESULT_OUT_OF_MEMORY;
    }

    size_t path_len = strlen(path);
    if (path_len == 0 || path_len >= sizeof(ctx->path)) {
        free(ctx);
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    memcpy(ctx->path, path, path_len + 1);
    ctx->flags = flags;
    ctx->is_open = false;

    out_stream->ops = &s_audio_file_ops;
    out_stream->ctx = ctx;

    return AUDIO_RESULT_SUCCESS;
}

static inline audio_result_t audio_file_stream_is_valid(audio_stream_t *stream) {
    if (stream == NULL || stream->ctx == NULL) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    return AUDIO_RESULT_SUCCESS;
}

static audio_result_t audio_file_stream_open(audio_stream_t *stream) {
    RESULT_CHECK(audio_file_stream_is_valid(stream), AUDIO_RESULT_SUCCESS);

    audio_file_stream_ctx_t *ctx = (audio_file_stream_ctx_t *)stream->ctx;
    if (ctx->is_open) {
        return AUDIO_RESULT_SUCCESS;
    }

    FRESULT fr = f_open(&ctx->file, ctx->path, FA_READ);
    if (fr != FR_OK) {
        return AUDIO_RESULT_FAILED_TO_OPEN_FILE;
    }

    ctx->is_open = true;
    return AUDIO_RESULT_SUCCESS;
}

static audio_result_t audio_file_stream_close(audio_stream_t *stream) {
    RESULT_CHECK(audio_file_stream_is_valid(stream), AUDIO_RESULT_SUCCESS);

    audio_file_stream_ctx_t *ctx = (audio_file_stream_ctx_t *)stream->ctx;

    if (ctx->is_open) {
        (void)f_close(&ctx->file);
        ctx->is_open = false;
    }

    free(ctx);
    stream->ctx = NULL;
    stream->ops = NULL;
    return AUDIO_RESULT_SUCCESS;
}

static audio_result_t audio_file_stream_read(audio_stream_t *stream, audio_buffer_t *buffer) {
    RESULT_CHECK(audio_file_stream_is_valid(stream), AUDIO_RESULT_SUCCESS);

    audio_file_stream_ctx_t *ctx = (audio_file_stream_ctx_t *)stream->ctx;

    if (!ctx->is_open) {
        audio_result_t r = audio_file_stream_open(stream);
        if (r != AUDIO_RESULT_SUCCESS) {
            return r;
        }
    }

    const size_t bps = audio_format_raw_bytes_per_sample(ctx->flags);
    if (bps == 0) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    const size_t max_bytes = audio_format_max_raw_bytes(buffer->max_size, ctx->flags);
    if (max_bytes == 0) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    UINT bytes_read = 0;
    FRESULT fr = f_read(&ctx->file, buffer->buffer, max_bytes, &bytes_read);
    if (fr != FR_OK) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    if (bytes_read == 0) {
        buffer->size = 0;
        return AUDIO_RESULT_SUCCESS;
    }

    bytes_read -= bytes_read % bps;

    buffer->size = bytes_read / bps;

    audio_decoder_unpack_inplace(buffer, ctx->flags);

    return AUDIO_RESULT_SUCCESS;
}

audio_result_t audio_file_stream_reset(audio_stream_t *stream) {
    RESULT_CHECK(audio_file_stream_is_valid(stream), AUDIO_RESULT_SUCCESS);

    audio_file_stream_ctx_t *ctx = (audio_file_stream_ctx_t *)stream->ctx;

    if (!ctx->is_open) {
        return audio_file_stream_open(stream);
    }

    FRESULT fr = f_lseek(&ctx->file, 0);
    if (fr != FR_OK) {
        return AUDIO_RESULT_INVALID_ARGUMENT;
    }

    return AUDIO_RESULT_SUCCESS;
}