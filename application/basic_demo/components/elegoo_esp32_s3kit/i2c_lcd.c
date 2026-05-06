/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdlib.h>
#include <string.h>
#include "esp_check.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/i2c_master.h"
#include "i2c_lcd.h"

static const char *TAG = "i2c_lcd";

#define I2C_LCD_DEFAULT_ADDR       0x3C
#define I2C_LCD_DEFAULT_SCL_SPEED  400000
#define I2C_LCD_DEFAULT_CMD_BITS   8
#define I2C_LCD_DEFAULT_PARAM_BITS 8

i2c_lcd_handle_t *i2c_lcd_new_handle(const i2c_lcd_config_t *config, i2c_master_bus_handle_t i2c_bus)
{
    ESP_LOGI(TAG, "Initializing I2C LCD display");

    if (config == NULL || i2c_bus == NULL) {
        ESP_LOGE(TAG, "invalid config or I2C bus");
        return NULL;
    }

    i2c_lcd_handle_t *handle = calloc(1, sizeof(i2c_lcd_handle_t));
    ESP_GOTO_ON_FALSE(handle != NULL, NULL, err, TAG, "failed to allocate memory");

    // 使用默认配置（如果未提供）
    uint32_t i2c_addr = (config->i2c_addr == 0) ? I2C_LCD_DEFAULT_ADDR : config->i2c_addr;
    uint32_t scl_speed = (config->scl_speed_hz == 0) ? I2C_LCD_DEFAULT_SCL_SPEED : config->scl_speed_hz;
    uint8_t cmd_bits = (config->lcd_cmd_bits == 0) ? I2C_LCD_DEFAULT_CMD_BITS : config->lcd_cmd_bits;
    uint8_t param_bits = (config->lcd_param_bits == 0) ? I2C_LCD_DEFAULT_PARAM_BITS : config->lcd_param_bits;

    // I2C LCD 配置
    const esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = i2c_addr,
        .lcd_cmd_bits = cmd_bits,
        .lcd_param_bits = param_bits,
        .scl_speed_hz = scl_speed,
        .flags = {
            .disable_control_phase = false,
        }
    };

    // 创建 I2C 面板 IO
    esp_err_t ret = esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &handle->io_handle);
    ESP_GOTO_ON_ERROR(ret, err_free, TAG, "failed to create I2C panel IO");

    // ST7789 面板配置
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = config->reset_gpio_num,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
        .bits_per_pixel = 16,
        .flags = {
            .reset_active_high = false,
        },
        .vendor_config = NULL,
    };

    // 创建 ST7789 面板
    ret = esp_lcd_new_panel_st7789(handle->io_handle, &panel_config, &handle->panel_handle);
    ESP_GOTO_ON_ERROR(ret, err_del_io, TAG, "failed to create ST7789 panel");

    // 初始化面板
    ret = esp_lcd_panel_init(handle->panel_handle);
    ESP_GOTO_ON_ERROR(ret, err_del_panel, TAG, "failed to initialize panel");

    // 设置显示参数
    (void)esp_lcd_panel_mirror(handle->panel_handle, config->mirror_x, config->mirror_y);
    (void)esp_lcd_panel_swap_xy(handle->panel_handle, config->swap_xy);
    (void)esp_lcd_panel_invert_color(handle->panel_handle, config->invert_color);

    // 打开显示
    (void)esp_lcd_panel_disp_on_off(handle->panel_handle, true);

    ESP_LOGI(TAG, "I2C LCD initialized successfully");
    return handle;

err_del_panel:
    esp_lcd_panel_del(handle->panel_handle);
err_del_io:
    esp_lcd_panel_io_del(handle->io_handle);
err_free:
    free(handle);
err:
    return NULL;
}

void i2c_lcd_delete_handle(i2c_lcd_handle_t *handle)
{
    if (handle == NULL) {
        return;
    }

    if (handle->panel_handle) {
        esp_lcd_panel_del(handle->panel_handle);
        handle->panel_handle = NULL;
    }
    if (handle->io_handle) {
        esp_lcd_panel_io_del(handle->io_handle);
        handle->io_handle = NULL;
    }

    free(handle);
    ESP_LOGI(TAG, "I2C LCD deinitialized successfully");
}