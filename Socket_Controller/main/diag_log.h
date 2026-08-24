#ifndef DIAG_LOG_H
#define DIAG_LOG_H

#include <stdbool.h>

/**
 * @brief Initialize the diagnostic logging system.
 * Must be called early in app_main() to catch early boot crashes.
 */
void diag_log_init(void);

/**
 * @brief Start the periodic heap monitoring task.
 * Call this after the main system is up and running.
 */
void diag_log_start_heap_monitor(void);

/**
 * @brief Enable or disable sending logs to the TCP stream queue.
 */
void diag_log_set_stream_active(bool active);

/**
 * @brief Retrieve the last crash logs from the RTC NOINIT buffer.
 * @return A dynamically allocated string containing the crash log. 
 *         The caller is responsible for freeing this memory.
 */
char* diag_log_get_crash_buffer_alloc(void);

#endif // DIAG_LOG_H