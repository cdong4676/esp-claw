/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdlib.h>
#include "esp_log.h"
#include "esp_check.h"
#include "esp_board_manager_includes.h"
#include "gen_board_device_custom.h"
#include "i2c_lcd.h"

static const char *TAG = "setup_device";

typedef struct {
    i2c_lcd_handle_t *lcd_handle;
} custom_i2c_lcd_device_t;

static int i2c_lcd_init(void *config, int cfg_size, void **device_handle)
{
    ESP_LOGI(TAG, "Initializing I2C LCD display via custom device");
    ESP_RETURN_ON_FALSE(device_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "invalid device handle");

    // 获取 I2C 外设句柄
    i2c_master_bus_handle_t i2c_bus = NULL;
    int ret = esp_board_manager_get_periph_handle("i2c_master", (void **)&i2c_bus);
    ESP_GOTO_ON_FALSE(ret == ESP_OK && i2c_bus != NULL, ESP_ERR_NOT_FOUND, err, TAG, "failed to get I2C peripheral handle");

    // 从 YAML 配置中获取 LCD 参数
    dev_custom_display_lcd_config_t *cfg = (dev_custom_display_lcd_config_t *)config;

    // LCD 配置
    const i2c_lcd_config_t lcd_config = {
        .i2c_addr = cfg->i2c_addr,
        .scl_speed_hz = cfg->scl_speed_hz,
        .lcd_cmd_bits = cfg->lcd_cmd_bits,
        .lcd_param_bits = cfg->lcd_param_bits,
        .reset_gpio_num = cfg->reset_gpio_num,
        .mirror_x = cfg->mirror_x,
        .mirror_y = cfg->mirror_y,
        .swap_xy = cfg->swap_xy,
        .invert_color = cfg->invert_color,
        .x_max = cfg->x_max,
        .y_max = cfg->y_max,
    };

    // 创建 LCD 句柄
    i2c_lcd_handle_t *lcd_handle = i2c_lcd_new_handle(&lcd_config, i2c_bus);
    ESP_GOTO_ON_FALSE(lcd_handle != NULL, ESP_ERR_NO_MEM, err, TAG, "failed to create I2C LCD handle");

    // 创建设备结构体
    custom_i2c_lcd_device_t *device = calloc(1, sizeof(custom_i2c_lcd_device_t));
    ESP_GOTO_ON_FALSE(device != NULL, ESP_ERR_NO_MEM, err_del_lcd, TAG, "failed to allocate device memory");

    device->lcd_handle = lcd_handle;
    *device_handle = device;

    ESP_LOGI(TAG, "I2C LCD custom device initialized successfully");
    return ESP_OK;

err_del_lcd:
    i2c_lcd_delete_handle(lcd_handle);
err:
    return ESP_FAIL;
}

static int i2c_lcd_deinit(void *device_handle)
{
    ESP_RETURN_ON_FALSE(device_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "invalid device handle");

    custom_i2c_lcd_device_t *device = (custom_i2c_lcd_device_t *)device_handle;

    if (device->lcd_handle) {
        i2c_lcd_delete_handle(device->lcd_handle);
    }

    free(device);
    ESP_LOGI(TAG, "I2C LCD custom device deinitialized successfully");
    return ESP_OK;
}

CUSTOM_DEVICE_IMPLEMENT(display_lcd, i2c_lcd_init, i2c_lcd_deinit);