/*
 * Elegoo ESP32-S3 SuperKit I2S Audio Codec
 * Direct I2S interface without hardware codec
 */
#include <stdlib.h>
#include <string.h>
#include "esp_check.h"
#include "esp_log.h"
#include "esp_codec_dev.h"
#include "i2s_audio_codec.h"

static const char *TAG = "i2s_audio_codec";

typedef struct {
    audio_codec_if_t base;
    i2s_audio_codec_config_t config;
    bool is_open;
    bool is_enabled;
} i2s_audio_codec_t;

typedef struct {
    audio_codec_data_if_t base;
    i2s_audio_codec_t *codec;
} i2s_audio_codec_data_t;

static int i2s_codec_open(const audio_codec_if_t *h, void *cfg, int cfg_size)
{
    (void)cfg;
    (void)cfg_size;
    i2s_audio_codec_t *codec = (i2s_audio_codec_t *)h;
    if (codec == NULL) {
        return ESP_CODEC_DEV_INVALID_ARG;
    }
    codec->is_open = true;
    ESP_LOGI(TAG, "I2S audio codec opened (%s)", codec->config.is_input ? "input" : "output");
    return ESP_CODEC_DEV_OK;
}

static bool i2s_codec_is_open(const audio_codec_if_t *h)
{
    const i2s_audio_codec_t *codec = (const i2s_audio_codec_t *)h;
    return codec != NULL && codec->is_open;
}

static int i2s_codec_enable(const audio_codec_if_t *h, bool enable)
{
    i2s_audio_codec_t *codec = (i2s_audio_codec_t *)h;
    if (codec == NULL) {
        return ESP_CODEC_DEV_INVALID_ARG;
    }
    codec->is_enabled = enable;
    ESP_LOGI(TAG, "I2S audio codec %s (%s)", enable ? "enabled" : "disabled", 
             codec->config.is_input ? "input" : "output");
    return ESP_CODEC_DEV_OK;
}

static int i2s_codec_set_fs(const audio_codec_if_t *h, esp_codec_dev_sample_info_t *fs)
{
    (void)h;
    (void)fs;
    return ESP_CODEC_DEV_OK;
}

static int i2s_codec_mute(const audio_codec_if_t *h, bool mute)
{
    (void)h;
    (void)mute;
    return ESP_CODEC_DEV_NOT_SUPPORT;
}

static int i2s_codec_set_vol(const audio_codec_if_t *h, float db)
{
    (void)h;
    (void)db;
    return ESP_CODEC_DEV_NOT_SUPPORT;
}

static int i2s_codec_set_mic_gain(const audio_codec_if_t *h, float db)
{
    (void)h;
    (void)db;
    return ESP_CODEC_DEV_NOT_SUPPORT;
}

static int i2s_codec_set_mic_channel_gain(const audio_codec_if_t *h, uint16_t channel_mask, float db)
{
    (void)h;
    (void)channel_mask;
    (void)db;
    return ESP_CODEC_DEV_NOT_SUPPORT;
}

static int i2s_codec_mute_mic(const audio_codec_if_t *h, bool mute)
{
    (void)h;
    (void)mute;
    return ESP_CODEC_DEV_NOT_SUPPORT;
}

static int i2s_codec_set_reg(const audio_codec_if_t *h, int reg, int value)
{
    (void)h;
    (void)reg;
    (void)value;
    return ESP_CODEC_DEV_NOT_SUPPORT;
}

static int i2s_codec_get_reg(const audio_codec_if_t *h, int reg, int *value)
{
    (void)h;
    (void)reg;
    (void)value;
    return ESP_CODEC_DEV_NOT_SUPPORT;
}

static void i2s_codec_dump_reg(const audio_codec_if_t *h)
{
    (void)h;
}

static int i2s_codec_close(const audio_codec_if_t *h)
{
    i2s_audio_codec_t *codec = (i2s_audio_codec_t *)h;
    if (codec == NULL) {
        return ESP_CODEC_DEV_INVALID_ARG;
    }
    codec->is_open = false;
    codec->is_enabled = false;
    ESP_LOGI(TAG, "I2S audio codec closed (%s)", codec->config.is_input ? "input" : "output");
    return ESP_CODEC_DEV_OK;
}

static int i2s_data_open(const audio_codec_data_if_t *h, void *data_cfg, int cfg_size)
{
    (void)data_cfg;
    (void)cfg_size;
    i2s_audio_codec_data_t *data_if = (i2s_audio_codec_data_t *)h;
    if (data_if == NULL || data_if->codec == NULL) {
        return ESP_CODEC_DEV_INVALID_ARG;
    }
    return ESP_CODEC_DEV_OK;
}

static bool i2s_data_is_open(const audio_codec_data_if_t *h)
{
    const i2s_audio_codec_data_t *data_if = (const i2s_audio_codec_data_t *)h;
    return data_if != NULL && data_if->codec != NULL;
}

static int i2s_data_enable(const audio_codec_data_if_t *h, esp_codec_dev_type_t dev_type, bool enable)
{
    (void)dev_type;
    i2s_audio_codec_data_t *data_if = (i2s_audio_codec_data_t *)h;
    if (data_if == NULL || data_if->codec == NULL) {
        return ESP_CODEC_DEV_INVALID_ARG;
    }
    data_if->codec->is_enabled = enable;
    return ESP_CODEC_DEV_OK;
}

static int i2s_data_set_fmt(const audio_codec_data_if_t *h, esp_codec_dev_type_t dev_type,
                            esp_codec_dev_sample_info_t *fs)
{
    (void)dev_type;
    (void)h;
    (void)fs;
    return ESP_CODEC_DEV_OK;
}

static int i2s_data_read(const audio_codec_data_if_t *h, uint8_t *data, int size)
{
    (void)h;
    (void)data;
    (void)size;
    return ESP_CODEC_DEV_NOT_SUPPORT;
}

static int i2s_data_write(const audio_codec_data_if_t *h, uint8_t *data, int size)
{
    (void)h;
    (void)data;
    (void)size;
    return ESP_CODEC_DEV_NOT_SUPPORT;
}

static int i2s_data_close(const audio_codec_data_if_t *h)
{
    i2s_audio_codec_data_t *data_if = (i2s_audio_codec_data_t *)h;
    if (data_if == NULL || data_if->codec == NULL) {
        return ESP_CODEC_DEV_INVALID_ARG;
    }
    data_if->codec->is_enabled = false;
    return ESP_CODEC_DEV_OK;
}

static const audio_codec_if_t *i2s_codec_new_codec_if(const i2s_audio_codec_config_t *config)
{
    i2s_audio_codec_t *codec = calloc(1, sizeof(*codec));
    if (codec == NULL) {
        return NULL;
    }

    codec->base.open = i2s_codec_open;
    codec->base.is_open = i2s_codec_is_open;
    codec->base.enable = i2s_codec_enable;
    codec->base.set_fs = i2s_codec_set_fs;
    codec->base.mute = i2s_codec_mute;
    codec->base.set_vol = i2s_codec_set_vol;
    codec->base.set_mic_gain = i2s_codec_set_mic_gain;
    codec->base.set_mic_channel_gain = i2s_codec_set_mic_channel_gain;
    codec->base.mute_mic = i2s_codec_mute_mic;
    codec->base.set_reg = i2s_codec_set_reg;
    codec->base.get_reg = i2s_codec_get_reg;
    codec->base.dump_reg = i2s_codec_dump_reg;
    codec->base.close = i2s_codec_close;
    codec->config = *config;
    codec->base.open(&codec->base, NULL, 0);

    return &codec->base;
}

static const audio_codec_data_if_t *i2s_codec_new_data_if(const audio_codec_if_t *codec_if)
{
    i2s_audio_codec_data_t *data_if = calloc(1, sizeof(*data_if));
    if (data_if == NULL) {
        return NULL;
    }

    data_if->base.open = i2s_data_open;
    data_if->base.is_open = i2s_data_is_open;
    data_if->base.enable = i2s_data_enable;
    data_if->base.set_fmt = i2s_data_set_fmt;
    data_if->base.read = i2s_data_read;
    data_if->base.write = i2s_data_write;
    data_if->base.close = i2s_data_close;
    data_if->codec = (i2s_audio_codec_t *)codec_if;

    return &data_if->base;
}

dev_audio_codec_handles_t *i2s_audio_codec_new_handle(const i2s_audio_codec_config_t *config)
{
    if (config == NULL) {
        return NULL;
    }

    dev_audio_codec_handles_t *codec_handles = calloc(1, sizeof(*codec_handles));
    if (codec_handles == NULL) {
        return NULL;
    }
    codec_handles->tx_aux_out_io = -1;

    codec_handles->codec_if = i2s_codec_new_codec_if(config);
    if (codec_handles->codec_if == NULL) {
        free(codec_handles);
        return NULL;
    }

    codec_handles->data_if = i2s_codec_new_data_if(codec_handles->codec_if);
    if (codec_handles->data_if == NULL) {
        audio_codec_delete_codec_if(codec_handles->codec_if);
        free(codec_handles);
        return NULL;
    }

    esp_codec_dev_cfg_t dev_cfg = {
        .codec_if = codec_handles->codec_if,
        .data_if = codec_handles->data_if,
        .dev_type = config->is_input ? ESP_CODEC_DEV_TYPE_IN : ESP_CODEC_DEV_TYPE_OUT,
    };
    codec_handles->codec_dev = esp_codec_dev_new(&dev_cfg);
    if (codec_handles->codec_dev == NULL) {
        audio_codec_delete_data_if(codec_handles->data_if);
        audio_codec_delete_codec_if(codec_handles->codec_if);
        free(codec_handles);
        return NULL;
    }

    ESP_LOGI(TAG, "I2S audio codec handle created (%s, %" PRIu32 " Hz)", 
             config->is_input ? "input" : "output", config->sample_rate);
    return codec_handles;
}

void i2s_audio_codec_delete_handle(dev_audio_codec_handles_t *codec_handles)
{
    if (codec_handles == NULL) {
        return;
    }
    if (codec_handles->codec_dev) {
        esp_codec_dev_delete(codec_handles->codec_dev);
        codec_handles->codec_dev = NULL;
    }
    if (codec_handles->data_if) {
        audio_codec_delete_data_if(codec_handles->data_if);
        codec_handles->data_if = NULL;
    }
    if (codec_handles->codec_if) {
        audio_codec_delete_codec_if(codec_handles->codec_if);
        codec_handles->codec_if = NULL;
    }
    free(codec_handles);
    ESP_LOGI(TAG, "I2S audio codec handle deleted");
}