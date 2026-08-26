#include "display_manager.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"


static const char *TAG = "DISP_MGR";

static SemaphoreHandle_t lvgl_mux = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

/* ---------------------------------------------------------------------------
 * LVGL Flush Callback (Synchronous Mode)
 *
 * Sends the rendered draw buffer to the ST7789 over SPI, then immediately
 * signals LVGL that the flush is done. This is the simplest, most robust
 * approach — no DMA lifecycle management needed. LVGL's 10ms task sleep
 * already de-couples rendering from the network tasks on Core 0.
 * ------------------------------------------------------------------------- */
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_handle_t panel = (esp_lcd_panel_handle_t)drv->user_data;
    esp_lcd_panel_draw_bitmap(panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
    lv_disp_flush_ready(drv); /* Signal LVGL: buffer consumed, ready for next frame */
}

/* ---------------------------------------------------------------------------
 * LVGL Tick Timer Callback (fires every 2 ms via esp_timer)
 * ------------------------------------------------------------------------- */
static void lvgl_tick_timer_cb(void *arg) {
    lv_tick_inc(2);
}

/* ---------------------------------------------------------------------------
 * Mutex helpers — callers must wrap all LVGL API calls with these
 * ------------------------------------------------------------------------- */
bool display_manager_lock(uint32_t timeout_ms) {
    if (!lvgl_mux) return false;
    return xSemaphoreTake(lvgl_mux, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void display_manager_unlock(void) {
    if (lvgl_mux) {
        xSemaphoreGive(lvgl_mux);
    }
}

/* ---------------------------------------------------------------------------
 * LVGL Port Task — pinned to Core 1 (keeps display rendering off Core 0
 * which handles Wi-Fi / LwIP / MicroLink network tasks)
 * ------------------------------------------------------------------------- */
static void lvgl_port_task(void *arg) {
    ESP_LOGI(TAG, "LVGL port task started on Core 1");
    while (1) {
        if (display_manager_lock(10)) {
            lv_timer_handler();
            display_manager_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* ---------------------------------------------------------------------------
 * display_manager_init
 *
 * Brings up the full display stack:
 *   1. SPI bus (DMA-capable)
 *   2. esp_lcd Panel IO (SPI abstraction layer)
 *   3. ST7789 vendor panel driver (reset, init, landscape orientation)
 *   4. LVGL init + DMA draw buffer + display driver registration
 *   5. esp_timer tick source (2 ms)
 *   6. FreeRTOS LVGL port task
 * ------------------------------------------------------------------------- */
esp_err_t display_manager_init(void) {
    lvgl_mux = xSemaphoreCreateMutex();
    if (!lvgl_mux) return ESP_ERR_NO_MEM;

    /* ── 1. SPI Bus ────────────────────────────────────────────────────── */
    spi_bus_config_t buscfg = {
        .sclk_io_num     = PIN_LCD_SCK,
        .mosi_io_num     = PIN_LCD_MOSI,
        .miso_io_num     = -1,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = LCD_H_RES * LCD_DRAW_BUF_LINES * sizeof(lv_color_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "SPI2 bus initialized (DMA enabled)");

    /* ── 2. Panel IO (SPI transport abstraction) ───────────────────────── */
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num       = PIN_LCD_DC,
        .cs_gpio_num       = PIN_LCD_CS,
        .pclk_hz           = 40 * 1000 * 1000, /* 40 MHz */
        .lcd_cmd_bits      = 8,
        .lcd_param_bits    = 8,
        .spi_mode          = 0,
        .trans_queue_depth = 10,
        /* NOTE: on_color_trans_done is NOT set here. In ESP-IDF v5.x the
         * callback must be registered via esp_lcd_panel_io_register_event_callbacks()
         * AFTER the display driver is created. We use the synchronous flush path
         * so this callback is not needed. */
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));

    /* ── 3. ST7789 Panel Driver ────────────────────────────────────────── */
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_LCD_RST,
        .rgb_endian     = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    /* Landscape orientation: swap X/Y axes (90° rotation).
     * mirror_x=true is the standard correction for most 240×320 ST7789
     * modules in landscape. Adjust mirror args if the image appears
     * horizontally flipped on your specific module. */
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));

    /* ST7789 typically requires color inversion to render correct colors */
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    ESP_LOGI(TAG, "ST7789 panel initialized (landscape 320×240)");

    /* ── 4. LVGL Init + Draw Buffer + Display Driver ───────────────────── */
    lv_init();

    /* Allocate draw buffer in internal SRAM with DMA capability.
     * Size: 320 × 30 lines × 2 bytes (RGB565) = 19.2 KB */
    lv_color_t *buf1 = heap_caps_malloc(
        LCD_H_RES * LCD_DRAW_BUF_LINES * sizeof(lv_color_t),
        MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA
    );
    assert(buf1 != NULL && "Failed to allocate LVGL draw buffer in internal DMA SRAM");
    ESP_LOGI(TAG, "LVGL draw buffer allocated: %u bytes in internal DMA SRAM",
             (unsigned)(LCD_H_RES * LCD_DRAW_BUF_LINES * sizeof(lv_color_t)));

    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf1, NULL, LCD_H_RES * LCD_DRAW_BUF_LINES);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res  = LCD_H_RES;
    disp_drv.ver_res  = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_drv_register(&disp_drv);
    ESP_LOGI(TAG, "LVGL display driver registered (%d×%d)", LCD_H_RES, LCD_V_RES);

    /* ── 5. LVGL Tick Timer (2 ms hardware timer) ──────────────────────── */
    const esp_timer_create_args_t tick_timer_args = {
        .callback = &lvgl_tick_timer_cb,
        .name     = "lvgl_tick",
    };
    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tick_timer_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 2000)); /* 2 ms = 2000 µs */

    /* ── 6. LVGL Port Task (Core 1, 8 KB stack) ────────────────────────── */
    BaseType_t ret = xTaskCreatePinnedToCore(
        lvgl_port_task, "lvgl_port",
        8192,       /* 8 KB — LVGL timer_handler needs headroom for animations */
        NULL, 5,    /* Priority 5 */
        NULL, 1     /* Core 1 */
    );
    assert(ret == pdPASS && "Failed to create LVGL port task");

    return ESP_OK;
}