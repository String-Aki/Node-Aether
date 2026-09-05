/**
 * display_controller.c
 *
 * Wires /screen/N, /screen/next, /slide/on[/N], /slide/off HTTP endpoints
 * onto the existing esp_httpd instance from web_server.c.
 *
 * Thread model: httpd handlers write to volatile flags; the LVGL
 * timer callback (single-threaded) reads them. No mutexes needed
 * as long as int-width flag reads/writes are atomic on Xtensa LX7.
 */
#include "display_controller.h"

#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "DISP_CTRL";

/* ── Shared flags (httpd task writes, LVGL timer task reads) ─────────────── */
static volatile int  s_screen_request  = -1;
static volatile int  s_current_screen  = 0;
static volatile int  s_slide_active    = 0;    /* 0 = off */
static volatile int  s_slide_interval  = 5000; /* ms, default 5 s */

#define SCREEN_COUNT 5
static const char *SCREEN_NAMES[SCREEN_COUNT] = {
    "Network", "Relay", "System Health", "Memory & Power", "Live Log"
};

/* ── Public API (LVGL thread) ───────────────────────────────────────────── */

int display_controller_poll_screen_request(void) {
    int req = s_screen_request;
    if (req >= 0) s_screen_request = -1;
    return req;
}

void display_controller_set_current_screen(int idx) {
    if (idx >= 0 && idx < SCREEN_COUNT) s_current_screen = idx;
}

void display_controller_get_slide_config(bool *active, int *interval_ms) {
    *active      = (s_slide_active != 0);
    *interval_ms = s_slide_interval;
}

/* ── HTTP helpers ───────────────────────────────────────────────────────── */

/** Send a unified JSON status response. */
static esp_err_t send_status(httpd_req_t *req, int screen) {
    char json[512];
    snprintf(json, sizeof(json),
        "{"
          "\"screen\":%d,"
          "\"name\":\"%s\","
          "\"slide\":{\"active\":%s,\"interval_s\":%d},"
          "\"screens\":["
            "\"0:Network\","
            "\"1:Relay\","
            "\"2:System Health\","
            "\"3:Memory & Power\","
            "\"4:Live Log\""
          "],"
          "\"endpoints\":["
            "\"/screen/{0-4}\","
            "\"/screen/next\","
            "\"/slide/on\","
            "\"/slide/on/{seconds}\","
            "\"/slide/off\""
          "]"
        "}",
        screen,
        (screen >= 0 && screen < SCREEN_COUNT) ? SCREEN_NAMES[screen] : "unknown",
        s_slide_active ? "true" : "false",
        s_slide_interval / 1000
    );
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_sendstr(req, json);
}

/* GET /screen        → status
   GET /screen/N      → jump to screen N
   GET /screen/next   → advance one screen             */
static esp_err_t screen_handler(httpd_req_t *req) {
    /* Extract the path suffix after /screen */
    const char *uri = req->uri;          /* e.g. "/screen/2" */
    const char *suffix = uri + 7;        /* skip "/screen" */

    int target = s_current_screen;

    if (*suffix == '\0' || strcmp(suffix, "/") == 0) {
        /* /screen — status only, no change */
    } else if (strcmp(suffix, "/next") == 0) {
        target           = (s_current_screen + 1) % SCREEN_COUNT;
        s_screen_request = target;
    } else {
        /* /screen/N */
        int n = atoi(suffix + 1);   /* skip leading '/' */
        if (n >= 0 && n < SCREEN_COUNT) {
            target           = n;
            s_screen_request = target;
        } else {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid screen index");
            return ESP_OK;
        }
    }

    ESP_LOGI(TAG, "Screen request → %d (%s)", target,
             (target >= 0 && target < SCREEN_COUNT) ? SCREEN_NAMES[target] : "?");
    return send_status(req, target);
}

/* GET /slide/on[/N]  → enable auto-slide (N seconds, 1-60)
   GET /slide/off     → disable auto-slide
   GET /slide         → status                                */
static esp_err_t slide_handler(httpd_req_t *req) {
    const char *suffix = req->uri + 6;   /* skip "/slide" */

    if (*suffix == '\0' || strcmp(suffix, "/") == 0) {
        /* status only */
    } else if (strcmp(suffix, "/off") == 0) {
        s_slide_active = 0;
        ESP_LOGI(TAG, "Auto-slide OFF");
    } else if (strncmp(suffix, "/on", 3) == 0) {
        s_slide_active = 1;
        if (suffix[3] == '/') {
            int secs = atoi(suffix + 4);
            if (secs >= 1 && secs <= 60) s_slide_interval = secs * 1000;
        } else {
            s_slide_interval = 5000;  /* default 5 s */
        }
        ESP_LOGI(TAG, "Auto-slide ON, interval=%d ms", s_slide_interval);
    } else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Unknown slide command");
        return ESP_OK;
    }

    return send_status(req, s_current_screen);
}

/* ── Init: register URI handlers on the provided httpd instance ─────────── */

void display_controller_init(httpd_handle_t server) {
    if (!server) {
        ESP_LOGE(TAG, "display_controller_init: null httpd handle");
        return;
    }

    /* Wildcard handler for /screen and all sub-paths */
    httpd_uri_t uri_screen = {
        .uri      = "/screen*",
        .method   = HTTP_GET,
        .handler  = screen_handler,
        .user_ctx = NULL,
    };
    /* Wildcard handler for /slide and all sub-paths */
    httpd_uri_t uri_slide = {
        .uri      = "/slide*",
        .method   = HTTP_GET,
        .handler  = slide_handler,
        .user_ctx = NULL,
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &uri_screen));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &uri_slide));

    ESP_LOGI(TAG, "Display control endpoints registered on existing httpd:");
    ESP_LOGI(TAG, "  GET /screen, /screen/{0-4}, /screen/next");
    ESP_LOGI(TAG, "  GET /slide/on[/{N}], /slide/off");
}
