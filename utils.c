#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* Cross-platform strcasecmp */
static int safe_strcasecmp(const char *s1, const char *s2) {
    if (s1 == NULL || s2 == NULL) {
        return (s1 == s2) ? 0 : (s1 == NULL ? -1 : 1);
    }
    while (*s1 && *s2) {
        int diff = tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
        if (diff != 0) {
            return diff;
        }
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

DateTime utils_get_current_time(void) {
    time_t raw_time = time(NULL);
    struct tm *info = localtime(&raw_time);
    DateTime dt = {0};

    if (info != NULL) {
        dt.year   = info->tm_year + 1900;
        dt.month  = info->tm_mon + 1;
        dt.day    = info->tm_mday;
        dt.hour   = info->tm_hour;
        dt.minute = info->tm_min;
        dt.second = info->tm_sec;
    }
    return dt;
}

void utils_format_datetime(DateTime dt, char *buffer, size_t max_len) {
    if (buffer == NULL || max_len == 0) {
        return;
    }
    snprintf(buffer, max_len, "%04d-%02d-%02d %02d:%02d:%02d",
             dt.year, dt.month, dt.day,
             dt.hour, dt.minute, dt.second);
}

static time_t datetime_to_epoch(DateTime dt) {
    struct tm tm_info;
    memset(&tm_info, 0, sizeof(struct tm));
    tm_info.tm_year = dt.year - 1900;
    tm_info.tm_mon  = dt.month - 1;
    tm_info.tm_mday = dt.day;
    tm_info.tm_hour = dt.hour;
    tm_info.tm_min  = dt.minute;
    tm_info.tm_sec  = dt.second;
    tm_info.tm_isdst = -1; /* Auto-detect daylight saving */

    return mktime(&tm_info);
}

int utils_hours_difference(DateTime earlier, DateTime later) {
    time_t t1 = datetime_to_epoch(earlier);
    time_t t2 = datetime_to_epoch(later);

    if (t1 == (time_t)-1 || t2 == (time_t)-1) {
        return 0;
    }

    double seconds = difftime(t2, t1);
    return (int)(seconds / 3600.0);
}

bool utils_is_older_than_hours(DateTime dt, int threshold_hours) {
    DateTime now = utils_get_current_time();
    int elapsed = utils_hours_difference(dt, now);
    return (elapsed >= threshold_hours);
}

void utils_trim_whitespace(char *str) {
    if (str == NULL) {
        return;
    }

    /* Trim leading spaces */
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *str = '\0';
        return;
    }

    /* Trim trailing spaces */
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

void utils_safe_strcpy(char *dest, const char *src, size_t dest_size) {
    if (dest == NULL || dest_size == 0) {
        return;
    }
    if (src == NULL) {
        dest[0] = '\0';
        return;
    }
    snprintf(dest, dest_size, "%s", src);
    dest[dest_size - 1] = '\0';
}

SystemResult utils_read_line(char *buffer, size_t max_len) {
    if (buffer == NULL || max_len == 0) {
        return SYS_ERR_NULL_PTR;
    }

    if (fgets(buffer, (int)max_len, stdin) == NULL) {
        buffer[0] = '\0';
        return SYS_ERR_INVALID_INPUT;
    }

    /* Check if newline was captured */
    size_t len = strlen(buffer);
    if (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
            buffer[--len] = '\0';
        }
    } else if (len == max_len - 1) {
        /* Buffer was full and newline was left in stream: flush remaining chars */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* discard */
        }
    }

    utils_trim_whitespace(buffer);
    return SYS_OK;
}

SystemResult utils_read_int(int min_val, int max_val, int *out_val) {
    if (out_val == NULL) {
        return SYS_ERR_NULL_PTR;
    }

    char buffer[64];
    while (1) {
        SystemResult res = utils_read_line(buffer, sizeof(buffer));
        if (res != SYS_OK || buffer[0] == '\0') {
            printf("  [!] Please enter a valid number: ");
            continue;
        }

        char *endptr = NULL;
        long val = strtol(buffer, &endptr, 10);
        if (endptr == buffer || *endptr != '\0') {
            printf("  [!] Invalid numeric format. Try again: ");
            continue;
        }

        if (val < min_val || val > max_val) {
            printf("  [!] Out of range [%d - %d]. Try again: ", min_val, max_val);
            continue;
        }

        *out_val = (int)val;
        return SYS_OK;
    }
}

const char* utils_status_to_string(ComplaintStatus status) {
    switch (status) {
        case STATUS_OPEN:        return "OPEN";
        case STATUS_ASSIGNED:    return "ASSIGNED";
        case STATUS_IN_PROGRESS: return "IN_PROGRESS";
        case STATUS_ESCALATED:   return "ESCALATED";
        case STATUS_RESOLVED:    return "RESOLVED";
        case STATUS_CLOSED:      return "CLOSED";
        default:                 return "UNKNOWN";
    }
}

SystemResult utils_string_to_status(const char *str, ComplaintStatus *out_status) {
    if (str == NULL || out_status == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    if (safe_strcasecmp(str, "OPEN") == 0) {
        *out_status = STATUS_OPEN;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "ASSIGNED") == 0) {
        *out_status = STATUS_ASSIGNED;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "IN_PROGRESS") == 0 || safe_strcasecmp(str, "IN PROGRESS") == 0) {
        *out_status = STATUS_IN_PROGRESS;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "ESCALATED") == 0) {
        *out_status = STATUS_ESCALATED;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "RESOLVED") == 0) {
        *out_status = STATUS_RESOLVED;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "CLOSED") == 0) {
        *out_status = STATUS_CLOSED;
        return SYS_OK;
    }
    return SYS_ERR_INVALID_INPUT;
}

const char* utils_priority_to_string(ComplaintPriority priority) {
    switch (priority) {
        case PRIORITY_LOW:      return "LOW";
        case PRIORITY_MEDIUM:   return "MEDIUM";
        case PRIORITY_HIGH:     return "HIGH";
        case PRIORITY_CRITICAL: return "CRITICAL";
        default:                return "UNKNOWN";
    }
}

SystemResult utils_string_to_priority(const char *str, ComplaintPriority *out_priority) {
    if (str == NULL || out_priority == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    if (safe_strcasecmp(str, "LOW") == 0) {
        *out_priority = PRIORITY_LOW;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "MEDIUM") == 0) {
        *out_priority = PRIORITY_MEDIUM;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "HIGH") == 0) {
        *out_priority = PRIORITY_HIGH;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "CRITICAL") == 0) {
        *out_priority = PRIORITY_CRITICAL;
        return SYS_OK;
    }
    return SYS_ERR_INVALID_INPUT;
}

const char* utils_category_to_string(ComplaintCategory cat) {
    switch (cat) {
        case CAT_ACADEMIC:       return "ACADEMIC";
        case CAT_HOSTEL:         return "HOSTEL";
        case CAT_INFRASTRUCTURE: return "INFRASTRUCTURE";
        case CAT_FINANCE:        return "FINANCE";
        case CAT_EXAMINATION:    return "EXAMINATION";
        case CAT_OTHER:          return "OTHER";
        default:                 return "UNKNOWN";
    }
}

SystemResult utils_string_to_category(const char *str, ComplaintCategory *out_cat) {
    if (str == NULL || out_cat == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    if (safe_strcasecmp(str, "ACADEMIC") == 0) {
        *out_cat = CAT_ACADEMIC;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "HOSTEL") == 0) {
        *out_cat = CAT_HOSTEL;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "INFRASTRUCTURE") == 0) {
        *out_cat = CAT_INFRASTRUCTURE;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "FINANCE") == 0) {
        *out_cat = CAT_FINANCE;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "EXAMINATION") == 0) {
        *out_cat = CAT_EXAMINATION;
        return SYS_OK;
    } else if (safe_strcasecmp(str, "OTHER") == 0) {
        *out_cat = CAT_OTHER;
        return SYS_OK;
    }
    return SYS_ERR_INVALID_INPUT;
}

const char* utils_result_to_string(SystemResult res) {
    switch (res) {
        case SYS_OK:                     return "Operation successful";
        case SYS_ERR_NULL_PTR:           return "Null pointer encountered";
        case SYS_ERR_NOT_FOUND:          return "Requested entity not found";
        case SYS_ERR_FILE_IO:            return "File I/O operation failed";
        case SYS_ERR_INVALID_INPUT:      return "Invalid or malformed input";
        case SYS_ERR_DUPLICATE:          return "Duplicate identifier";
        case SYS_ERR_NO_MEMORY:          return "Memory allocation failure";
        case SYS_ERR_INVALID_TRANSITION: return "Invalid status transition";
        case SYS_ERR_LIMIT_REACHED:      return "System capacity limit reached";
        default:                         return "Unknown error";
    }
}

void utils_pause_prompt(void) {
    printf("\nPress [Enter] to continue...");
    char dummy[16];
    utils_read_line(dummy, sizeof(dummy));
}

void utils_clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[H\033[J");
#endif
}
