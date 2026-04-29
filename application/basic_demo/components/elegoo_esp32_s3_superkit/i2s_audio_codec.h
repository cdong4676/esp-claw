/*
 * Elegoo ESP32-S3 SuperKit I2S Audio Codec
 * Direct I2S interface without hardware codec
 */
#pragma once

#include <stdbool.h>
#include "dev_audio_codec.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t sample_rate;
    bool is_input;
} i2s_audio_codec_config_t;

dev_audio_codec_handles_t *i2s_audio_codec_new_handle(const i2s_audio_codec_config_t *config);
void i2s_audio_codec_delete_handle(dev_audio_codec_handles_t *codec_handles);

#ifdef __cplusplus
}
#endif