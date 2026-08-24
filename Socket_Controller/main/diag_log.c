#include "diag_log.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "lwip/sockets.h"

#define TAG "DIAG_LOG"

// --- RTC Crash Buffer Configuration ---
#define RTC_MAGIC_WORD 0x55AA55AA
#define CRASH_BUF_SIZE 2048

// RTC_NOINIT_ATTR ensures these survive software crashes and resets
RTC_NOINIT_ATTR static uint32_t s_rtc_magic;
RTC_NOINIT_ATTR static uint32_t s_rtc_head;
RTC_NOINIT_ATTR static char s_rtc_buf[CRASH_BUF_SIZE];

// --- Streaming & Queue Configuration ---
#define LOG_QUEUE_SIZE 40
#define LOG_MSG_MAX_LEN 128

typedef struct {
    char text[LOG_MSG_MAX_LEN];
} log_msg_t;

static QueueHandle_t s_log_queue = NULL;
static vprintf_like_t s_orig_vprintf = NULL;
static volatile bool s_stream_active = false;

// --- VPrintf Hook ---
static int diag_vprintf(const char *fmt, va_list args) {
    char buf[LOG_MSG_MAX_LEN];
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    
    // 1. Write synchronously to RTC Ring Buffer (Crash Survival)
    for (int i = 0; i < len; i++) {
        s_rtc_buf[s_rtc_head] = buf[i];
        s_rtc_head = (s_rtc_head + 1) % CRASH_BUF_SIZE;
    }

    // 2. Push to Network Queue (if streaming is ON)
    if (s_stream_active && s_log_queue) {
        log_msg_t msg;
        strncpy(msg.text, buf, sizeof(msg.text) - 1);
        msg.text[sizeof(msg.text) - 1] = '\0';
        
        if (xPortInIsrContext()) {
            BaseType_t high_task_woken = pdFALSE;
            xQueueSendFromISR(s_log_queue, &msg, &high_task_woken);
        } else {
            // Non-blocking push, drop if full to avoid stalling tasks
            xQueueSend(s_log_queue, &msg, 0); 
        }
    }

    // 3. Send to standard UART output
    return s_orig_vprintf(fmt, args);
}

// --- TCP Log Sender Task ---
static void log_relay_task(void *arg) {
    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(9002) // Dedicated port for live log stream
    };

    bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(listen_sock, 1);

    ESP_LOGI(TAG, "Log relay server listening on port 9002");

    while (1) {
        struct sockaddr_in source_addr;
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        
        if (sock < 0) continue;
        ESP_LOGI(TAG, "Log client connected!");

        // Send a welcome message
        const char* welcome = "--- MicroLink Live Log Stream ---\n";
        send(sock, welcome, strlen(welcome), 0);

        log_msg_t msg;
        while (1) {
            if (xQueueReceive(s_log_queue, &msg, pdMS_TO_TICKS(1000)) == pdTRUE) {
                int written = send(sock, msg.text, strlen(msg.text), 0);
                if (written < 0) {
                    ESP_LOGI(TAG, "Log client disconnected");
                    break; 
                }
            }
        }
        close(sock);
    }
}

// --- Heap Monitor Task ---
static void heap_monitor_task(void *arg) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(60000)); // Every 60s
        uint32_t free_heap = esp_get_free_heap_size();
        uint32_t min_free = esp_get_minimum_free_heap_size();
        uint32_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        uint32_t max_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
        
        // Log at WARN level so it stands out in standard terminal output
        ESP_LOGW("HEAP", "free_int=%lu, free_psram=%lu, min_free=%lu, largest_block=%lu",
                 (unsigned long)free_heap, (unsigned long)free_psram, 
                 (unsigned long)min_free, (unsigned long)max_block);
    }
}

// --- Public APIs ---

void diag_log_init(void) {
    // Check if magic word matches to detect if RTC RAM is uninitialized
    if (s_rtc_magic != RTC_MAGIC_WORD) {
        memset(s_rtc_buf, 0, CRASH_BUF_SIZE);
        s_rtc_head = 0;
        s_rtc_magic = RTC_MAGIC_WORD;
    } else if (esp_reset_reason() == ESP_RST_PANIC) {
        // If we just recovered from a crash, print the final logs to UART
        ESP_LOGE(TAG, "CRASH DETECTED! Previous RTC Buffer Contents:");
        char *crash_dump = diag_log_get_crash_buffer_alloc();
        if (crash_dump) {
            printf("\n--- BEGIN CRASH DUMP ---\n%s\n--- END CRASH DUMP ---\n", crash_dump);
            free(crash_dump);
        }
    }

    s_log_queue = xQueueCreate(LOG_QUEUE_SIZE, sizeof(log_msg_t));
    s_orig_vprintf = esp_log_set_vprintf(diag_vprintf);

    // REMOVED log_relay_task creation from here.
}

void diag_log_start_heap_monitor(void) {
    xTaskCreate(heap_monitor_task, "heap_monitor", 4096, NULL, 4, NULL);
    
    // ADDED log_relay_task creation here (after networking is initialized)
    xTaskCreate(log_relay_task, "log_relay", 3072, NULL, 5, NULL);
}

void diag_log_set_stream_active(bool active) {
    s_stream_active = active;
}

char* diag_log_get_crash_buffer_alloc(void) {
    char *dump = malloc(CRASH_BUF_SIZE + 1);
    if (!dump) return NULL;

    // Unwind the circular buffer
    int pos = 0;
    for (int i = 0; i < CRASH_BUF_SIZE; i++) {
        uint32_t idx = (s_rtc_head + i) % CRASH_BUF_SIZE;
        char c = s_rtc_buf[idx];
        // Skip null characters from the initial empty buffer
        if (c != '\0') {
            dump[pos++] = c;
        }
    }
    dump[pos] = '\0';
    return dump;
}