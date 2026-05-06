/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdbool.h>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t i2c_addr;          /*!< I2C device address */
    uint32_t scl_speed_hz;      /*!< I2C clock speed */
    uint8_t lcd_cmd_bits;       /*!< LCD command bits */
    uint8_t lcd_param_bits;     /*!< LCD parameter bits */
    int reset_gpio_num;         /*!< Reset GPIO number, -1 if not used */
    bool mirror_x;              /*!< Mirror display horizontally */
    bool mirror_y;              /*!< Mirror display vertically */
    bool swap_xy;               /*!< Swap X and Y coordinates */
    bool invert_color;          /*!< Invert display colors */
    uint16_t x_max;             /*!< Maximum X resolution */
    uint16_t y_max;             /*!< Maximum Y resolution */
} i2c_lcd_config_t;

typedef struct {
    esp_lcd_panel_io_handle_t io_handle;
    esp_lcd_panel_handle_t panel_handle;
} i2c_lcd_handle_t;

i2c_lcd_handle_t *i2c_lcd_new_handle(const i2c_lcd_config_t *config, i2c_master_bus_handle_t i2c_bus);
void i2c_lcd_delete_handle(i2c_lcd_handle_t *handle);

#ifdef __cplusplus
}
#endif