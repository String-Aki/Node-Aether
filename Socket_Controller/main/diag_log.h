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

/**
 * @brief Fill `out` with the last `n` log lines from the live RTC ring buffer.
 *        Each element in `out` is a null-terminated string of up to `line_len-1` chars.
 *        Lines are ordered oldest-first; out[n-1] is the newest (bottom of display).
 *        Call from any task — reads the ring buffer safely.
 * @param out      2-D char array, caller-owned. Dimensions: [n][line_len].
 * @param n        Number of lines to fill (matches LOG_SCREEN_LINES = 10).
 * @param line_len Max bytes per line including null terminator.
 */
void diag_log_get_last_n_lines(char out[][80], int n, int line_len);

#endif // DIAG_LOG_H