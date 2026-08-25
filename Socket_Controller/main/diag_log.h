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
 * @brief Retrieve the FROZEN crash snapshot from the last unexpected reset.
 *        Unlike the live buffer, this does not change between calls.
 *        Returns a string with reset reason + the log tail at crash time.
 * @return A dynamically allocated string containing the crash snapshot.
 *         The caller is responsible for freeing this memory.
 *         Returns NULL if no crash snapshot has been recorded.
 */
char* diag_log_get_last_crash_snapshot_alloc(void);

/**
 * @brief Retrieve the last N lines from the LIVE ring buffer (changes on every refresh).
 *        Useful for viewing real-time tail during debugging, but NOT a crash log.
 * @return A dynamically allocated string containing the live buffer tail.
 *         The caller is responsible for freeing this memory.
 */
char* diag_log_get_crash_buffer_alloc(void);

#endif // DIAG_LOG_H