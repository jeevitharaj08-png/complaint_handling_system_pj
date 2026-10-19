#ifndef UTILS_H
#define UTILS_H

#include "common.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Retrieves the current system calendar date and time.
 * @return DateTime populated with current local time.
 */
DateTime utils_get_current_time(void);

/**
 * @brief Formats a DateTime struct into a standardized string: "YYYY-MM-DD HH:MM:SS".
 * @param dt The DateTime structure to format.
 * @param buffer Output buffer.
 * @param max_len Maximum capacity of the output buffer.
 */
void utils_format_datetime(DateTime dt, char *buffer, size_t max_len);

/**
 * @brief Calculates difference in hours between two DateTime values.
 * @return Number of whole hours elapsed from earlier to later. Negative if later < earlier.
 */
int utils_hours_difference(DateTime earlier, DateTime later);

/**
 * @brief Checks if a given timestamp is older than threshold hours compared to current system time.
 * @param dt Timestamp to check.
 * @param threshold_hours Number of hours.
 * @return true if elapsed hours >= threshold_hours, false otherwise.
 */
bool utils_is_older_than_hours(DateTime dt, int threshold_hours);

/**
 * @brief Trims leading and trailing whitespace, carriage returns, and newlines in place.
 * @param str Null-terminated string to modify.
 */
void utils_trim_whitespace(char *str);

/**
 * @brief Safe string copy that guarantees null-termination and prevents buffer overflow.
 * @param dest Destination buffer.
 * @param src Source string.
 * @param dest_size Size of destination buffer.
 */
void utils_safe_strcpy(char *dest, const char *src, size_t dest_size);

/**
 * @brief Safely reads a line of text from stdin, stripping trailing newlines and flushing any overflow.
 * @param buffer Output string buffer.
 * @param max_len Capacity of buffer.
 * @return SYS_OK on success, SYS_ERR_INVALID_INPUT on empty input or error.
 */
SystemResult utils_read_line(char *buffer, size_t max_len);

/**
 * @brief Safely reads an integer within [min_val, max_val] from stdin with re-prompting.
 * @param min_val Minimum acceptable value.
 * @param max_val Maximum acceptable value.
 * @param out_val Pointer to store the parsed integer.
 * @return SYS_OK on success.
 */
SystemResult utils_read_int(int min_val, int max_val, int *out_val);

/**
 * @brief String representation of a ComplaintStatus enum.
 */
const char* utils_status_to_string(ComplaintStatus status);

/**
 * @brief Converts string to ComplaintStatus. Case-insensitive.
 */
SystemResult utils_string_to_status(const char *str, ComplaintStatus *out_status);

/**
 * @brief String representation of a ComplaintPriority enum.
 */
const char* utils_priority_to_string(ComplaintPriority priority);

/**
 * @brief Converts string to ComplaintPriority. Case-insensitive.
 */
SystemResult utils_string_to_priority(const char *str, ComplaintPriority *out_priority);

/**
 * @brief String representation of a ComplaintCategory enum.
 */
const char* utils_category_to_string(ComplaintCategory cat);

/**
 * @brief Converts string to ComplaintCategory. Case-insensitive.
 */
SystemResult utils_string_to_category(const char *str, ComplaintCategory *out_cat);

/**
 * @brief Human-readable error message string for a SystemResult code.
 */
const char* utils_result_to_string(SystemResult res);

/**
 * @brief Pauses execution and waits for user to press Enter.
 */
void utils_pause_prompt(void);

/**
 * @brief Clears console screen in a cross-platform manner.
 */
void utils_clear_screen(void);

#endif /* UTILS_H */
