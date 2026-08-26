#include "display_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fonts/ui_fonts.h"
#include "lvgl.h"
#include <stdio.h>

static const char *TAG = "FONT_SHOWCASE";

void app_main(void) {
  ESP_LOGI(TAG, "=== 1-BIT BITMAP FONT SHOWCASE ===");

  ESP_ERROR_CHECK(display_manager_init());

  if (display_manager_lock(1000)) {
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x081018), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Header bar — Silkscreen 12 used here, grain-free on cyan fill */
    lv_obj_t *hdr = lv_obj_create(scr);
    lv_obj_set_size(hdr, 320, 28);
    lv_obj_set_pos(hdr, 0, 0);
    lv_obj_set_style_bg_color(hdr, lv_color_hex(0x08A6D6), 0);
    lv_obj_set_style_border_width(hdr, 0, 0);
    lv_obj_set_style_radius(hdr, 0, 0);
    lv_obj_set_style_pad_all(hdr, 0, 0);
    lv_obj_t *hdr_lbl = lv_label_create(hdr);
    lv_label_set_text(hdr_lbl, "BITMAP FONT SHOWCASE  1BPP  ZERO GRAIN");
    lv_obj_set_style_text_font(hdr_lbl, &ui_font_silkscreen_12, 0);
    lv_obj_set_style_text_color(hdr_lbl, lv_color_hex(0x081018), 0);
    lv_obj_align(hdr_lbl, LV_ALIGN_CENTER, 0, 0);

    /* Colors that were grainy with Montserrat AA — bitmap should be clean */
    lv_color_t cyan  = lv_color_hex(0x00FFFF);
    lv_color_t mint  = lv_color_hex(0x00FF99);
    lv_color_t white = lv_color_hex(0xFFFFFF);
    lv_color_t gray  = lv_color_hex(0x4A6080);

    int y = 36;
    lv_obj_t *l, *d;

    /* ── Orbitron ───────────────────────────────────────────── */
    l = lv_label_create(scr);
    lv_label_set_text(l, "Orbitron 16  NODE AETHER 192.168.1.1");
    lv_obj_set_style_text_font(l, &ui_font_orbitron_16, 0);
    lv_obj_set_style_text_color(l, cyan, 0);
    lv_obj_set_pos(l, 6, y); y += 22;

    l = lv_label_create(scr);
    lv_label_set_text(l, "Orbitron 12  STATUS: ONLINE");
    lv_obj_set_style_text_font(l, &ui_font_orbitron_12, 0);
    lv_obj_set_style_text_color(l, mint, 0);
    lv_obj_set_pos(l, 6, y); y += 20;

    d = lv_obj_create(scr);
    lv_obj_set_size(d, 320, 1); lv_obj_set_pos(d, 0, y); y += 6;
    lv_obj_set_style_bg_color(d, lv_color_hex(0x1A2A3A), 0);
    lv_obj_set_style_border_width(d, 0, 0); lv_obj_set_style_radius(d, 0, 0);

    /* ── Share Tech Mono ────────────────────────────────────── */
    l = lv_label_create(scr);
    lv_label_set_text(l, "ShareTechMono 16  10.0.0.1:8080");
    lv_obj_set_style_text_font(l, &ui_font_share_tech_mono_16, 0);
    lv_obj_set_style_text_color(l, white, 0);
    lv_obj_set_pos(l, 6, y); y += 22;

    l = lv_label_create(scr);
    lv_label_set_text(l, "ShareTechMono 12  TX:34kbps  RX:112kbps");
    lv_obj_set_style_text_font(l, &ui_font_share_tech_mono_12, 0);
    lv_obj_set_style_text_color(l, gray, 0);
    lv_obj_set_pos(l, 6, y); y += 20;

    d = lv_obj_create(scr);
    lv_obj_set_size(d, 320, 1); lv_obj_set_pos(d, 0, y); y += 6;
    lv_obj_set_style_bg_color(d, lv_color_hex(0x1A2A3A), 0);
    lv_obj_set_style_border_width(d, 0, 0); lv_obj_set_style_radius(d, 0, 0);

    /* ── Silkscreen ─────────────────────────────────────────── */
    l = lv_label_create(scr);
    lv_label_set_text(l, "Silkscreen 16  RELAY: ON  TEMP: 42C");
    lv_obj_set_style_text_font(l, &ui_font_silkscreen_16, 0);
    lv_obj_set_style_text_color(l, cyan, 0);
    lv_obj_set_pos(l, 6, y); y += 22;

    l = lv_label_create(scr);
    lv_label_set_text(l, "Silkscreen 12  UPTIME: 14d 06h 23m");
    lv_obj_set_style_text_font(l, &ui_font_silkscreen_12, 0);
    lv_obj_set_style_text_color(l, mint, 0);
    lv_obj_set_pos(l, 6, y); y += 20;

    d = lv_obj_create(scr);
    lv_obj_set_size(d, 320, 1); lv_obj_set_pos(d, 0, y); y += 6;
    lv_obj_set_style_bg_color(d, lv_color_hex(0x1A2A3A), 0);
    lv_obj_set_style_border_width(d, 0, 0); lv_obj_set_style_radius(d, 0, 0);

    /* ── VT323 ──────────────────────────────────────────────── */
    l = lv_label_create(scr);
    lv_label_set_text(l, "VT323 20  SOCKET CONTROLLER v2.1");
    lv_obj_set_style_text_font(l, &ui_font_vt323_20, 0);
    lv_obj_set_style_text_color(l, mint, 0);
    lv_obj_set_pos(l, 6, y); y += 24;

    l = lv_label_create(scr);
    lv_label_set_text(l, "VT323 16  WireGuard: CONNECTED");
    lv_obj_set_style_text_font(l, &ui_font_vt323_16, 0);
    lv_obj_set_style_text_color(l, white, 0);
    lv_obj_set_pos(l, 6, y);

    display_manager_unlock();
  }

  ESP_LOGI(TAG, "Font showcase drawn. Idling forever.");
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}