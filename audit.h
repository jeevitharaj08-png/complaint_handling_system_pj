#ifndef AUDIT_H
#define AUDIT_H

#include "common.h"
#include <stddef.h>

/**
 * @brief In-memory singly linked list node representing an audit trail event.
 */
typedef struct AuditNode {
    DateTime timestamp;
    char complaint_id[MAX_ID_LEN];
    char action[MAX_ACTION_LEN];
    char actor[MAX_NAME_LEN];
    char remarks[MAX_NOTES_LEN];
    struct AuditNode *next;
} AuditNode;

/* Forward declaration for Complaint */
struct Complaint;

/**
 * @brief Appends an audit event to disk (data/audit.log) and attaches it to in-memory complaint history.
 * @param complaint Target complaint (can be NULL if logging global system events).
 * @param complaint_id Complaint ID string.
 * @param action Short action keyword (e.g. CREATED, ASSIGNED, ESCALATED, RESOLVED, CLOSED).
 * @param actor Name or ID of user/system executing the action.
 * @param remarks Descriptive details or reasons.
 */
void audit_record_event(AuditNode **history_head,
                        const char *complaint_id,
                        const char *action,
                        const char *actor,
                        const char *remarks);

/**
 * @brief Frees the entire linked list chain of audit nodes.
 */
void audit_free_chain(AuditNode *head);

/**
 * @brief Prints formatted timeline history of all events for a given complaint.
 */
void audit_print_history(const AuditNode *head, const char *complaint_id);

/**
 * @brief Reads and displays the latest lines from the persistent audit log file (data/audit.log).
 * @param max_lines Maximum number of trailing lines to display (0 for all).
 */
void audit_display_global_log(size_t max_lines);

#endif /* AUDIT_H */
