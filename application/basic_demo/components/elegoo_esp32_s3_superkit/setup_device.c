#include <string.h>
#include "esp_log.h"
#include "esp_lcd_panel_st7789.h"
#include "esp_board_manager_includes.h"
#include "i2s_audio_codec.h"
#include "gen_board_device_custom.h"

static const char *TAG = "setup_device";

esp_err_t lcd_panel_factory_entry_t(esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel)
{
    esp_lcd_panel_dev_config_t panel_dev_cfg = {0};
    memcpy(&panel_dev_cfg, panel_dev_config, sizeof(esp_lcd_panel_dev_config_t));
    int ret = esp_lcd_new_panel_st7789(io, &panel_dev_cfg, ret_panel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "New st7789 panel failed");
        return ret;
    }
    return ESP_OK;
}

static int audio_adc_init(void *config, int cfg_size, void **device_handle)
{
    ESP_LOGI(TAG, "Initializing audio_adc device");
    uint32_t sample_rate = 16000;
    
    if (config != NULL) {
        const dev_custom_audio_adc_config_t *cfg = (const dev_custom_audio_adc_config_t *)config;
        sample_rate = cfg->sample_rate_hz > 0 ? cfg->sample_rate_hz : 16000;
    }
    
    i2s_audio_codec_config_t codec_cfg = {
        .sample_rate = sample_rate,
        .is_input = true
    };
    
    *device_handle = i2s_audio_codec_new_handle(&codec_cfg);
    return (*device_handle != NULL) ? 0 : -1;
}

static int audio_adc_deinit(void *device_handle)
{
    ESP_LOGI(TAG, "Deinitializing audio_adc device");
    i2s_audio_codec_delete_handle((dev_audio_codec_handles_t *)device_handle);
    return 0;
}

static int audio_dac_init(void *config, int cfg_size, void **device_handle)
{
    ESP_LOGI(TAG, "Initializing audio_dac device");
    uint32_t sample_rate = 24000;
    
    if (config != NULL) {
        const dev_custom_audio_dac_config_t *cfg = (const dev_custom_audio_dac_config_t *)config;
        sample_rate = cfg->sample_rate_hz > 0 ? cfg->sample_rate_hz : 16000;
    }
    
    i2s_audio_codec_config_t codec_cfg = {
        .sample_rate = sample_rate,
        .is_input = false
    };
    
    *device_handle = i2s_audio_codec_new_handle(&codec_cfg);
    return (*device_handle != NULL) ? 0 : -1;
}

static int audio_dac_deinit(void *device_handle)
{
    ESP_LOGI(TAG, "Deinitializing audio_dac device");
    i2s_audio_codec_delete_handle((dev_audio_codec_handles_t *)device_handle);
    return 0;
}

CUSTOM_DEVICE_IMPLEMENT(audio_adc, audio_adc_init, audio_adc_deinit);
CUSTOM_DEVICE_IMPLEMENT(audio_dac, audio_dac_init, audio_dac_deinit);