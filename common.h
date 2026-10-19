#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* ========================================================================= */
/* System String & Buffer Limits                                             */
/* ========================================================================= */
#define MAX_ID_LEN         16
#define MAX_NAME_LEN       64
#define MAX_EMAIL_LEN      64
#define MAX_PHONE_LEN      20
#define MAX_ADDR_LEN      128
#define MAX_DESC_LEN      512
#define MAX_NOTES_LEN     256
#define MAX_ACTION_LEN     32
#define MAX_CATEGORY_LEN   32
#define MAX_LINE_BUFFER   1024

/* Default escalation threshold in hours for CRITICAL complaints (configurable) */
#define DEFAULT_CRITICAL_ESCALATION_HOURS 24

/* Storage File Paths */
#define DATA_DIR               "data"
#define FILE_COMPLAINTS_DAT    "data/complaints.dat"
#define FILE_CUSTOMERS_DAT     "data/customers.dat"
#define FILE_AGENTS_DAT        "data/agents.dat"
#define FILE_AUDIT_LOG         "data/audit.log"

/* Magic Numbers for File Headers */
#define MAGIC_COMPLAINTS       0x434D5053 /* "CMPS" */
#define MAGIC_CUSTOMERS        0x43555354 /* "CUST" */
#define MAGIC_AGENTS           0x41474E54 /* "AGNT" */
#define STORAGE_VERSION        1

/* ========================================================================= */
/* Enumerations                                                              */
/* ========================================================================= */

/**
 * @brief Complaint lifecycle status states.
 */
typedef enum {
    STATUS_OPEN = 0,
    STATUS_ASSIGNED,
    STATUS_IN_PROGRESS,
    STATUS_ESCALATED,
    STATUS_RESOLVED,
    STATUS_CLOSED,
    STATUS_COUNT
} ComplaintStatus;

/**
 * @brief Complaint priority levels.
 */
typedef enum {
    PRIORITY_LOW = 0,
    PRIORITY_MEDIUM,
    PRIORITY_HIGH,
    PRIORITY_CRITICAL,
    PRIORITY_COUNT
} ComplaintPriority;

/**
 * @brief College domain complaint categories.
 */
typedef enum {
    CAT_ACADEMIC = 0,
    CAT_HOSTEL,
    CAT_INFRASTRUCTURE,
    CAT_FINANCE,
    CAT_EXAMINATION,
    CAT_OTHER,
    CAT_COUNT
} ComplaintCategory;

/**
 * @brief Standardized system operation result codes.
 */
typedef enum {
    SYS_OK = 0,
    SYS_ERR_NULL_PTR = -1,
    SYS_ERR_NOT_FOUND = -2,
    SYS_ERR_FILE_IO = -3,
    SYS_ERR_INVALID_INPUT = -4,
    SYS_ERR_DUPLICATE = -5,
    SYS_ERR_NO_MEMORY = -6,
    SYS_ERR_INVALID_TRANSITION = -7,
    SYS_ERR_LIMIT_REACHED = -8
} SystemResult;

/* ========================================================================= */
/* Common Nested Data Structures                                             */
/* ========================================================================= */

/**
 * @brief Representation of calendar date and 24-hour time.
 */
typedef struct {
    int year;   /* e.g., 2026 */
    int month;  /* 1 - 12 */
    int day;    /* 1 - 31 */
    int hour;   /* 0 - 23 */
    int minute; /* 0 - 59 */
    int second; /* 0 - 59 */
} DateTime;

/**
 * @brief Resolution details embedded inside a Complaint structure.
 */
typedef struct {
    char resolved_by_agent_id[MAX_ID_LEN];
    DateTime resolved_at;
    char resolution_notes[MAX_NOTES_LEN];
    int satisfaction_rating; /* 1 - 5 stars, 0 if unrated */
} ResolutionInfo;

#endif /* COMMON_H */
