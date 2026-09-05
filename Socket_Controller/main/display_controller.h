/**
 * display_controller.h
 *
 * Node-Aether — ESP32-S3 display screen-switch adapter.
 * Mirrors the sim_http_server.h API so dashboard_ui.c can be
 * ported with a single #include swap.
 *
 * Registers /screen/{N|next} and /slide/{on|off} URI handlers
 * on the existing esp_httpd instance from web_server.c.
 * Uses volatile flags polled by the LVGL timer — no LVGL calls
 * from the httpd handler context.
 */
#pragma once
#include <stdbool.h>
#include <esp_http_server.h>

/**
 * Register /screen/N, /screen/next, /slide/on[/N], /slide/off URI handlers
 * on an already-started httpd instance. Call once after start_webserver() returns.
 */
void display_controller_init(httpd_handle_t server);

/**
 * Poll for a pending screen-switch request.
 * Returns -1 (no request) or 0-4 (screen index, one-shot consume).
 * Call from the LVGL main thread only (inside the 2 s timer callback).
 */
int display_controller_poll_screen_request(void);

/** Report which screen is currently active. Call after every screen switch. */
void display_controller_set_current_screen(int screen_idx);

/**
 * Read current auto-slide config.
 * *active = true if sliding is on, *interval_ms = ms between slides.
 * Call from the LVGL main thread only.
 */
void display_controller_get_slide_config(bool *active, int *interval_ms);
