#ifndef COMPLAINT_H
#define COMPLAINT_H

#include "common.h"
#include "audit.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Complaint entity structure (in-memory representation).
 */
typedef struct {
    char complaint_id[MAX_ID_LEN];
    char customer_id[MAX_ID_LEN];
    ComplaintCategory category;
    char description[MAX_DESC_LEN];
    ComplaintPriority priority;
    ComplaintStatus status;
    char assigned_agent_id[MAX_ID_LEN];
    DateTime created_at;
    DateTime updated_at;
    ResolutionInfo resolution;     /* Nested structure */
    AuditNode *audit_history_head; /* Singly linked list pointer */
} Complaint;

/**
 * @brief Doubly linked list node for complaints.
 */
typedef struct ComplaintNode {
    Complaint data;
    struct ComplaintNode *prev;
    struct ComplaintNode *next;
} ComplaintNode;

/**
 * @brief Complaint collection container managing head, tail, and metrics.
 */
typedef struct {
    ComplaintNode *head;
    ComplaintNode *tail;
    size_t count;
    uint32_t next_id_counter;
    int critical_escalation_hours; /* Configurable SLA threshold */
} ComplaintList;

/**
 * @brief Disk-serializable fixed-size record (without pointers).
 */
typedef struct {
    char complaint_id[MAX_ID_LEN];
    char customer_id[MAX_ID_LEN];
    int32_t category;
    char description[MAX_DESC_LEN];
    int32_t priority;
    int32_t status;
    char assigned_agent_id[MAX_ID_LEN];
    DateTime created_at;
    DateTime updated_at;
    ResolutionInfo resolution;
} ComplaintDiskRecord;

/**
 * @brief Initializes the doubly linked list and sets default configuration.
 */
void complaint_list_init(ComplaintList *list);

/**
 * @brief Frees all complaint nodes and their associated in-memory audit logs.
 */
void complaint_list_cleanup(ComplaintList *list);

/**
 * @brief Registers a new complaint and appends to the doubly linked list.
 * @param list Pointer to complaint list.
 * @param cust_id Customer ID submitting the complaint.
 * @param cat Domain category.
 * @param description Detailed text description.
 * @param priority Initial priority.
 * @param actor Name/Role of person initiating registration.
 * @param out_id Output buffer for generated complaint ID (`CMP-XXXX`).
 * @return SYS_OK on success.
 */
SystemResult complaint_register(ComplaintList *list,
                                const char *cust_id,
                                ComplaintCategory cat,
                                const char *description,
                                ComplaintPriority priority,
                                const char *actor,
                                char *out_id);

/**
 * @brief Finds a complaint node by unique Complaint ID.
 */
ComplaintNode* complaint_find_by_id(const ComplaintList *list, const char *id);

/**
 * @brief Assigns or reassigns a complaint to a specific agent manually.
 */
SystemResult complaint_assign_agent(ComplaintList *list,
                                    const char *id,
                                    const char *agent_id,
                                    const char *actor);

/**
 * @brief Automatically assigns a complaint to the least-loaded available agent.
 */
SystemResult complaint_auto_assign(ComplaintList *list,
                                   const char *id,
                                   const char *actor);

/**
 * @brief Advances a complaint status to IN_PROGRESS.
 */
SystemResult complaint_mark_in_progress(ComplaintList *list,
                                       const char *id,
                                       const char *actor,
                                       const char *notes);

/**
 * @brief Updates priority with an audit reason.
 */
SystemResult complaint_change_priority(ComplaintList *list,
                                       const char *id,
                                       ComplaintPriority new_priority,
                                       const char *actor,
                                       const char *reason);

/**
 * @brief Manually escalates a complaint with documented justification.
 */
SystemResult complaint_escalate_manual(ComplaintList *list,
                                       const char *id,
                                       const char *actor,
                                       const char *reason);

/**
 * @brief Scans for CRITICAL complaints older than the configured threshold (e.g. 24h)
 *        and escalates them automatically.
 * @return Count of complaints that were auto-escalated.
 */
size_t complaint_auto_escalate_critical(ComplaintList *list, const char *actor);

/**
 * @brief Resolves a complaint by providing resolution notes.
 */
SystemResult complaint_resolve(ComplaintList *list,
                               const char *id,
                               const char *agent_id,
                               const char *resolution_notes);

/**
 * @brief Closes a resolved complaint and records customer satisfaction rating (1 - 5).
 */
SystemResult complaint_close(ComplaintList *list,
                             const char *id,
                             int satisfaction_rating,
                             const char *actor);

/**
 * @brief Prints detailed card view of a single complaint.
 */
void complaint_print_details(const Complaint *c);

/**
 * @brief Formats and displays a tabular view of all complaints in the list.
 */
void complaint_display_all(const ComplaintList *list);

/**
 * @brief Converts in-memory Complaint to fixed-size DiskRecord.
 */
void complaint_to_disk_record(const Complaint *c, ComplaintDiskRecord *rec);

/**
 * @brief Loads a DiskRecord into the in-memory doubly linked list.
 */
SystemResult complaint_load_disk_record(ComplaintList *list, const ComplaintDiskRecord *rec);

#endif /* COMPLAINT_H */
