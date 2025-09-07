/**
 * @file
 * @brief lvgl_ui header file
 */

#pragma once
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"

#include "esp_lcd_touch_xpt2046.h"
#ifdef __cplusplus
extern "C"
{
#endif
    // C function declarations
    void lvgl_ui_init(void);

#ifdef __cplusplus
}
#endif
