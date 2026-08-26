#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "lvgl.h"

/* Landscape orientation: native 240×320 panel rotated 90° via swap_xy       */
#define LCD_H_RES              320
#define LCD_V_RES              240
#define LCD_DRAW_BUF_LINES     30   /* 320 × 30 × 2 = 19.2 KB internal DMA SRAM */

#define PIN_LCD_SCK            6
#define PIN_LCD_MOSI           7
#define PIN_LCD_CS             8
#define PIN_LCD_DC             15
#define PIN_LCD_RST            16
/* No BLK pin for 7-pin ST7789 module. Backlight is hardwired to VCC.        */

esp_err_t display_manager_init(void);
bool      display_manager_lock(uint32_t timeout_ms);
void      display_manager_unlock(void);