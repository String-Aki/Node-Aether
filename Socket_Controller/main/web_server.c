#include "web_server.h"
#include <esp_log.h>
#include <esp_system.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/temperature_sensor.h"

#include "relay.h"
#include "ethernet_wol.h"
#include "diag_log.h"

static const char *TAG = "WEB_SERVER";

static int phone_battery = -1;
static temperature_sensor_handle_t temp_sensor = NULL;

static const char HTML_DASHBOARD[] =
"<!DOCTYPE html><html lang='en'><head>"
"<meta charset='utf-8'>"
"<meta name='viewport' content='width=device-width,initial-scale=1'>"
"<meta name='theme-color' content='#121212'>"
"<title>Node Dashboard</title>"
"<style>"
"body{background:#121212;color:#eee;font-family:sans-serif;text-align:center;padding:15px;margin:0}"
".container{max-width:400px;margin:auto}"
"h2{margin:5px 0; color:#fff}"
".card{background:#1e1e1e;border-radius:12px;padding:15px;margin-bottom:15px;box-shadow:0 4px 6px rgba(0,0,0,0.3)}"
".status-row{display:flex;justify-content:space-between;align-items:center;padding:10px 0;border-bottom:1px solid #333}"
".status-row:last-child{border-bottom:none}"
".indicator{display:inline-block;width:12px;height:12px;border-radius:50%;margin-right:8px}"
".on{background:#34c759;box-shadow:0 0 8px #34c759}"
".off{background:#ff3b30;box-shadow:0 0 8px #ff3b30}"
"button{width:100%;border:none;border-radius:8px;padding:14px;font-size:15px;font-weight:bold;color:#fff;cursor:pointer;margin-bottom:10px}"
"button:active{opacity:0.8}"
".btn-boot{background:#007aff}"
".btn-on{background:#28a745}"
".btn-off{background:#dc3545}"
"#log{color:#aaa;font-size:13px;margin-top:10px;min-height:20px}"
"</style></head><body>"
"<div class='container'>"
"<h2>ESP32 Control Node</h2>"
"<div class='card'>"
"<h3>System Resources</h3>"
"<div class='status-row'><span>Internal RAM Free:</span><span id='s-int-ram'>...</span></div>"
"<div class='status-row'><span>PSRAM Free:</span><span id='s-psram'>...</span></div>"
"<div class='status-row'><span>Core Temp:</span><span id='s-temp'>...</span></div>"
"<div class='status-row'><span>Uptime:</span><span id='s-uptime'>...</span></div>"
"</div>"
"<div class='card'>"
"<div class='status-row'><span>Laptop Socket:</span><span id='s-r1'>...</span></div>"
"<div class='status-row'><span>Router Phone Socket:</span><span id='s-r2'>...</span></div>"
"<div class='status-row'><span>Router Phone Battery:</span><span id='s-bat'>...</span></div>"
"</div>"
"<div class='card'><h3>Laptop Controls</h3>"
"<button class='btn-boot' onclick='act(\"/laptop/boot\")'>Boot Laptop (Power Cycle)</button>"
"<button class='btn-on' onclick='act(\"/relay1/on\")'>Turn Socket ON</button>"
"<button class='btn-off' onclick='act(\"/relay1/off\")'>Turn Socket OFF</button>"
"</div>"
"<div class='card'><h3>Router Phone Controls</h3>"
"<button class='btn-on' onclick='act(\"/relay2/on\")'>Force Charger ON</button>"
"<button class='btn-off' onclick='act(\"/relay2/off\")'>Force Charger OFF</button>"
"</div>"
"<div class='card'><h3>Diagnostics</h3>"
"<button class='btn-boot' style='background:#f39c12;' onclick='window.open(\"/api/last_crash\", \"_blank\")'>View Last Crash Dump</button>"
"<div style='display:flex; gap:10px;'>"
"<button class='btn-on' onclick='act(\"/log/stream/on\")'>Log Stream ON</button>"
"<button class='btn-off' onclick='act(\"/log/stream/off\")'>Log Stream OFF</button>"
"</div>"
"</div>"
"<div id='log'>Ready.</div>"
"</div>"
"<script>"
"function act(url){"
" document.getElementById('log').innerText='Requesting...';"
" fetch(url).then(r=>r.text()).then(t=>{document.getElementById('log').innerText=t; updateStatus();});"
"}"
"function updateStatus() {"
"    fetch('/api/status').then(r=>r.json()).then(d=>{"
"        document.getElementById('s-r1').innerHTML = d.r1 ? '<span class=\"indicator on\"></span>ON' : '<span class=\"indicator off\"></span>OFF';"
"        document.getElementById('s-r2').innerHTML = d.r2 ? '<span class=\"indicator on\"></span>ON' : '<span class=\"indicator off\"></span>OFF';"
"        document.getElementById('s-bat').innerText = (d.bat>=0) ? d.bat+'%' : 'Waiting...';"
"    });"
"    fetch('/api/stats').then(r=>r.json()).then(d=>{"
"        document.getElementById('s-int-ram').innerText = (d.internal_free / 1024).toFixed(1) + ' KB';"
"        document.getElementById('s-psram').innerText = (d.psram_free / (1024 * 1024)).toFixed(2) + ' MB';"
"        document.getElementById('s-temp').innerText = d.temp.toFixed(1) + ' °C';"
"        let mins = Math.floor(d.uptime / 60);"
"        let secs = d.uptime % 60;"
"        document.getElementById('s-uptime').innerText = mins + 'm ' + secs + 's';"
"    }).catch(e => console.log('Stats error:', e));"
"}"
"setInterval(updateStatus, 5000); updateStatus();"
"</script></body></html>";

static esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    return httpd_resp_send(req, HTML_DASHBOARD, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t api_status_handler(httpd_req_t *req) {
    char json[100];
    snprintf(json, sizeof(json), "{\"r1\":%d, \"r2\":%d, \"bat\":%d}",
             relay_get_state(1) ? 1 : 0, relay_get_state(2) ? 1 : 0, phone_battery);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t api_stats_handler(httpd_req_t *req) {
    uint32_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    uint32_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    uint32_t uptime_sec = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);

    float core_temp = 0.0;
    if (temp_sensor != NULL) {
        temperature_sensor_get_celsius(temp_sensor, &core_temp);
    }

    char json[256];
    snprintf(json, sizeof(json),
             "{\"internal_free\":%lu, \"psram_free\":%lu, \"uptime\":%lu, \"temp\":%.1f}",
             (unsigned long)free_internal, (unsigned long)free_psram, (unsigned long)uptime_sec, core_temp);

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t api_battery_handler(httpd_req_t *req) {
    char buf[50];
    if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
        char val[10];
        if (httpd_query_key_value(buf, "level", val, sizeof(val)) == ESP_OK) {
            phone_battery = atoi(val);
        }
    }
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t laptop_boot_handler(httpd_req_t *req) {
    relay_set_state(1, false);
    vTaskDelay(pdMS_TO_TICKS(3000));
    relay_set_state(1, true);
    return httpd_resp_send(req, "Laptop Boot Cycle Triggered!", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t r1_on_handler(httpd_req_t *req) { relay_set_state(1, true); return httpd_resp_send(req, "R1 ON", -1); }
static esp_err_t r1_off_handler(httpd_req_t *req) { relay_set_state(1, false); return httpd_resp_send(req, "R1 OFF", -1); }
static esp_err_t r2_on_handler(httpd_req_t *req) { relay_set_state(2, true); return httpd_resp_send(req, "R2 ON", -1); }
static esp_err_t r2_off_handler(httpd_req_t *req) { relay_set_state(2, false); return httpd_resp_send(req, "R2 OFF", -1); }

// --- Diagnostic Endpoints ---
static esp_err_t log_stream_on_handler(httpd_req_t *req) {
    diag_log_set_stream_active(true);
    return httpd_resp_send(req, "TCP Log Stream ENABLED on port 9002", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t log_stream_off_handler(httpd_req_t *req) {
    diag_log_set_stream_active(false);
    return httpd_resp_send(req, "TCP Log Stream DISABLED", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t last_crash_handler(httpd_req_t *req) {
    char *dump = diag_log_get_crash_buffer_alloc();
    httpd_resp_set_type(req, "text/plain");
    
    if (dump && strlen(dump) > 0) {
        esp_err_t res = httpd_resp_send(req, dump, HTTPD_RESP_USE_STRLEN);
        free(dump);
        return res;
    } else {
        if (dump) free(dump);
        return httpd_resp_send(req, "No crash log found in RTC memory.", HTTPD_RESP_USE_STRLEN);
    }
}

httpd_handle_t start_webserver(void) {
    // Re-claim the Temperature Sensor for our custom dashboard
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 80);
    if (temperature_sensor_install(&temp_sensor_config, &temp_sensor) == ESP_OK) {
        temperature_sensor_enable(temp_sensor);
        ESP_LOGI(TAG, "Temperature sensor successfully claimed by Web Server!");
    } else {
        ESP_LOGW(TAG, "Failed to claim temperature sensor. Did MicroLink sneak in?");
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 8080;
    config.ctrl_port = 32769;
    config.max_uri_handlers = 16;
    config.lru_purge_enable = true;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_root = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler };
        httpd_uri_t uri_stat = { .uri = "/api/status", .method = HTTP_GET, .handler = api_status_handler };
        httpd_uri_t uri_bat  = { .uri = "/api/battery", .method = HTTP_GET, .handler = api_battery_handler };
        httpd_uri_t uri_boot = { .uri = "/laptop/boot", .method = HTTP_GET, .handler = laptop_boot_handler };
        httpd_uri_t uri_r1on = { .uri = "/relay1/on", .method = HTTP_GET, .handler = r1_on_handler };
        httpd_uri_t uri_r1of = { .uri = "/relay1/off", .method = HTTP_GET, .handler = r1_off_handler };
        httpd_uri_t uri_r2on = { .uri = "/relay2/on", .method = HTTP_GET, .handler = r2_on_handler };
        httpd_uri_t uri_r2of = { .uri = "/relay2/off", .method = HTTP_GET, .handler = r2_off_handler };
        httpd_uri_t uri_stats = { .uri = "/api/stats", .method = HTTP_GET, .handler = api_stats_handler };
        httpd_uri_t uri_log_on = { .uri = "/log/stream/on", .method = HTTP_GET, .handler = log_stream_on_handler };
        httpd_uri_t uri_log_off = { .uri = "/log/stream/off", .method = HTTP_GET, .handler = log_stream_off_handler };
        httpd_uri_t uri_last_crash = { .uri = "/api/last_crash", .method = HTTP_GET, .handler = last_crash_handler };

        httpd_register_uri_handler(server, &uri_root);
        httpd_register_uri_handler(server, &uri_stat);
        httpd_register_uri_handler(server, &uri_bat);
        httpd_register_uri_handler(server, &uri_boot);
        httpd_register_uri_handler(server, &uri_r1on);
        httpd_register_uri_handler(server, &uri_r1of);
        httpd_register_uri_handler(server, &uri_r2on);
        httpd_register_uri_handler(server, &uri_r2of);
        httpd_register_uri_handler(server, &uri_stats);
        httpd_register_uri_handler(server, &uri_log_on);
        httpd_register_uri_handler(server, &uri_log_off);
        httpd_register_uri_handler(server, &uri_last_crash);

        ESP_LOGI(TAG, "Web server started!");
        return server;
    }
    return NULL;
}

void stop_webserver(httpd_handle_t server) { if (server) httpd_stop(server); }