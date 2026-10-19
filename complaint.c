#include "complaint.h"
#include "agent.h"
#include "customer.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void complaint_list_init(ComplaintList *list) {
    if (list == NULL) {
        return;
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    list->next_id_counter = 1001;
    list->critical_escalation_hours = DEFAULT_CRITICAL_ESCALATION_HOURS;
}

void complaint_list_cleanup(ComplaintList *list) {
    if (list == NULL) {
        return;
    }

    ComplaintNode *curr = list->head;
    while (curr != NULL) {
        ComplaintNode *next = curr->next;
        /* Free nested in-memory audit log nodes */
        audit_free_chain(curr->data.audit_history_head);
        curr->data.audit_history_head = NULL;
        free(curr);
        curr = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    list->next_id_counter = 1001;
}

SystemResult complaint_register(ComplaintList *list,
                                const char *cust_id,
                                ComplaintCategory cat,
                                const char *description,
                                ComplaintPriority priority,
                                const char *actor,
                                char *out_id) {
    if (list == NULL || cust_id == NULL || description == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    if (strlen(cust_id) == 0 || strlen(description) == 0) {
        return SYS_ERR_INVALID_INPUT;
    }

    ComplaintNode *node = (ComplaintNode *)malloc(sizeof(ComplaintNode));
    if (node == NULL) {
        return SYS_ERR_NO_MEMORY;
    }
    memset(node, 0, sizeof(ComplaintNode));

    /* Populate ticket data */
    snprintf(node->data.complaint_id, sizeof(node->data.complaint_id), "CMP-%04u", list->next_id_counter++);
    utils_safe_strcpy(node->data.customer_id, cust_id, sizeof(node->data.customer_id));
    node->data.category = cat;
    utils_safe_strcpy(node->data.description, description, sizeof(node->data.description));
    node->data.priority = priority;
    node->data.status = STATUS_OPEN;
    node->data.assigned_agent_id[0] = '\0';
    node->data.created_at = utils_get_current_time();
    node->data.updated_at = node->data.created_at;
    memset(&node->data.resolution, 0, sizeof(ResolutionInfo));
    node->data.audit_history_head = NULL;

    /* Append to doubly linked list */
    node->prev = list->tail;
    node->next = NULL;

    if (list->tail != NULL) {
        list->tail->next = node;
    } else {
        list->head = node;
    }
    list->tail = node;
    list->count++;

    if (out_id != NULL) {
        utils_safe_strcpy(out_id, node->data.complaint_id, MAX_ID_LEN);
    }

    /* Record audit event */
    char remarks[128];
    snprintf(remarks, sizeof(remarks), "Complaint created [%s, %s]",
             utils_category_to_string(cat), utils_priority_to_string(priority));
    audit_record_event(&node->data.audit_history_head, node->data.complaint_id,
                       "CREATED", (actor != NULL) ? actor : "CUSTOMER", remarks);

    return SYS_OK;
}

ComplaintNode* complaint_find_by_id(const ComplaintList *list, const char *id) {
    if (list == NULL || id == NULL) {
        return NULL;
    }
    ComplaintNode *curr = list->head;
    while (curr != NULL) {
        if (strcmp(curr->data.complaint_id, id) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

SystemResult complaint_assign_agent(ComplaintList *list,
                                    const char *id,
                                    const char *agent_id,
                                    const char *actor) {
    if (list == NULL || id == NULL || agent_id == NULL) {
        return SYS_ERR_NULL_PTR;
    }

    ComplaintNode *node = complaint_find_by_id(list, id);
    if (node == NULL) {
        return SYS_ERR_NOT_FOUND;
    }

    SupportAgent *agent = agent_find_by_id(agent_id);
    if (agent == NULL) {
        return SYS_ERR_NOT_FOUND;
    }
    if (!agent->is_active) {
        return SYS_ERR_INVALID_INPUT;
    }
    if (node->data.status == STATUS_RESOLVED || node->data.status == STATUS_CLOSED) {
        return SYS_ERR_INVALID_TRANSITION;
    }

    /* Decrement previous agent workload if reassigning */
    if (strlen(node->data.assigned_agent_id) > 0) {
        agent_decrement_workload(node->data.assigned_agent_id);
    }

    /* Increment new agent workload */
    agent_increment_workload(agent_id);
    utils_safe_strcpy(node->data.assigned_agent_id, agent_id, sizeof(node->data.assigned_agent_id));

    if (node->data.status == STATUS_OPEN) {
        node->data.status = STATUS_ASSIGNED;
    }
    node->data.updated_at = utils_get_current_time();

    char remarks[128];
    snprintf(remarks, sizeof(remarks), "Assigned to %s (%s)", agent->full_name, agent->agent_id);
    audit_record_event(&node->data.audit_history_head, node->data.complaint_id,
                       "ASSIGNED", (actor != NULL) ? actor : "SUPERVISOR", remarks);

    return SYS_OK;
}

SystemResult complaint_auto_assign(ComplaintList *list,
                                   const char *id,
                                   const char *actor) {
    if (list == NULL || id == NULL) {
        return SYS_ERR_NULL_PTR;
    }

    ComplaintNode *node = complaint_find_by_id(list, id);
    if (node == NULL) {
        return SYS_ERR_NOT_FOUND;
    }

    /* Find least loaded agent matching complaint category */
    const char *dept = utils_category_to_string(node->data.category);
    SupportAgent *least_loaded = agent_find_least_loaded(dept);
    if (least_loaded == NULL) {
        /* Fall back to any department */
        least_loaded = agent_find_least_loaded(NULL);
    }

    if (least_loaded == NULL) {
        return SYS_ERR_NOT_FOUND;
    }

    return complaint_assign_agent(list, id, least_loaded->agent_id,
                                  (actor != NULL) ? actor : "AUTO-DISPATCH");
}

SystemResult complaint_mark_in_progress(ComplaintList *list,
                                       const char *id,
                                       const char *actor,
                                       const char *notes) {
    if (list == NULL || id == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    ComplaintNode *node = complaint_find_by_id(list, id);
    if (node == NULL) {
        return SYS_ERR_NOT_FOUND;
    }
    if (node->data.status == STATUS_RESOLVED || node->data.status == STATUS_CLOSED) {
        return SYS_ERR_INVALID_TRANSITION;
    }

    node->data.status = STATUS_IN_PROGRESS;
    node->data.updated_at = utils_get_current_time();

    audit_record_event(&node->data.audit_history_head, node->data.complaint_id,
                       "IN_PROGRESS", (actor != NULL) ? actor : "AGENT",
                       (notes != NULL && strlen(notes) > 0) ? notes : "Work started on ticket");

    return SYS_OK;
}

SystemResult complaint_change_priority(ComplaintList *list,
                                       const char *id,
                                       ComplaintPriority new_priority,
                                       const char *actor,
                                       const char *reason) {
    if (list == NULL || id == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    ComplaintNode *node = complaint_find_by_id(list, id);
    if (node == NULL) {
        return SYS_ERR_NOT_FOUND;
    }
    if (node->data.status == STATUS_CLOSED) {
        return SYS_ERR_INVALID_TRANSITION;
    }

    ComplaintPriority old_pr = node->data.priority;
    node->data.priority = new_priority;
    node->data.updated_at = utils_get_current_time();

    char remarks[128];
    snprintf(remarks, sizeof(remarks), "Priority changed from %s to %s. Reason: %s",
             utils_priority_to_string(old_pr),
             utils_priority_to_string(new_priority),
             (reason != NULL) ? reason : "Administrative update");

    audit_record_event(&node->data.audit_history_head, node->data.complaint_id,
                       "PRIORITY", (actor != NULL) ? actor : "OPERATOR", remarks);

    return SYS_OK;
}

SystemResult complaint_escalate_manual(ComplaintList *list,
                                       const char *id,
                                       const char *actor,
                                       const char *reason) {
    if (list == NULL || id == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    ComplaintNode *node = complaint_find_by_id(list, id);
    if (node == NULL) {
        return SYS_ERR_NOT_FOUND;
    }
    if (node->data.status == STATUS_RESOLVED ||
        node->data.status == STATUS_CLOSED ||
        node->data.status == STATUS_ESCALATED) {
        return SYS_ERR_INVALID_TRANSITION;
    }

    node->data.status = STATUS_ESCALATED;
    node->data.priority = PRIORITY_CRITICAL; /* Automatically elevate to CRITICAL */
    node->data.updated_at = utils_get_current_time();

    char remarks[128];
    snprintf(remarks, sizeof(remarks), "Escalated: %s",
             (reason != NULL && strlen(reason) > 0) ? reason : "Unresolved within standard timeframe");

    audit_record_event(&node->data.audit_history_head, node->data.complaint_id,
                       "ESCALATED", (actor != NULL) ? actor : "SUPERVISOR", remarks);

    return SYS_OK;
}

size_t complaint_auto_escalate_critical(ComplaintList *list, const char *actor) {
    if (list == NULL) {
        return 0;
    }

    DateTime now = utils_get_current_time();
    size_t escalated_count = 0;
    ComplaintNode *curr = list->head;

    while (curr != NULL) {
        Complaint *c = &curr->data;
        /* Only check unresolved and non-closed, non-escalated tickets */
        if (c->status != STATUS_RESOLVED &&
            c->status != STATUS_CLOSED &&
            c->status != STATUS_ESCALATED) {

            if (c->priority == PRIORITY_CRITICAL) {
                int elapsed = utils_hours_difference(c->created_at, now);
                if (elapsed >= list->critical_escalation_hours) {
                    c->status = STATUS_ESCALATED;
                    c->updated_at = now;

                    char remarks[128];
                    snprintf(remarks, sizeof(remarks),
                             "Auto-escalated: Critical SLA threshold (%d hrs) breached (elapsed: %d hrs)",
                             list->critical_escalation_hours, elapsed);

                    audit_record_event(&c->audit_history_head, c->complaint_id,
                                       "AUTO_ESCALATED",
                                       (actor != NULL) ? actor : "SLA-MONITOR",
                                       remarks);
                    escalated_count++;
                }
            }
        }
        curr = curr->next;
    }

    return escalated_count;
}

SystemResult complaint_resolve(ComplaintList *list,
                               const char *id,
                               const char *agent_id,
                               const char *resolution_notes) {
    if (list == NULL || id == NULL || resolution_notes == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    if (strlen(resolution_notes) == 0) {
        return SYS_ERR_INVALID_INPUT;
    }

    ComplaintNode *node = complaint_find_by_id(list, id);
    if (node == NULL) {
        return SYS_ERR_NOT_FOUND;
    }
    if (node->data.status == STATUS_RESOLVED || node->data.status == STATUS_CLOSED) {
        return SYS_ERR_INVALID_TRANSITION;
    }

    DateTime now = utils_get_current_time();
    node->data.status = STATUS_RESOLVED;
    node->data.updated_at = now;

    utils_safe_strcpy(node->data.resolution.resolved_by_agent_id,
                     (agent_id != NULL) ? agent_id : node->data.assigned_agent_id,
                     MAX_ID_LEN);
    node->data.resolution.resolved_at = now;
    utils_safe_strcpy(node->data.resolution.resolution_notes, resolution_notes,
                     sizeof(node->data.resolution.resolution_notes));

    /* Decrement agent's active ticket workload */
    if (strlen(node->data.assigned_agent_id) > 0) {
        agent_decrement_workload(node->data.assigned_agent_id);
    }

    audit_record_event(&node->data.audit_history_head, node->data.complaint_id,
                       "RESOLVED",
                       (agent_id != NULL) ? agent_id : "AGENT",
                       resolution_notes);

    return SYS_OK;
}

SystemResult complaint_close(ComplaintList *list,
                             const char *id,
                             int satisfaction_rating,
                             const char *actor) {
    if (list == NULL || id == NULL) {
        return SYS_ERR_NULL_PTR;
    }

    ComplaintNode *node = complaint_find_by_id(list, id);
    if (node == NULL) {
        return SYS_ERR_NOT_FOUND;
    }

    /* Enforce workflow rule: Must be in RESOLVED state before closing */
    if (node->data.status != STATUS_RESOLVED) {
        return SYS_ERR_INVALID_TRANSITION;
    }

    if (satisfaction_rating < 1 || satisfaction_rating > 5) {
        return SYS_ERR_INVALID_INPUT;
    }

    node->data.status = STATUS_CLOSED;
    node->data.resolution.satisfaction_rating = satisfaction_rating;
    node->data.updated_at = utils_get_current_time();

    char remarks[128];
    snprintf(remarks, sizeof(remarks), "Ticket closed by customer. Rating: %d/5 Stars", satisfaction_rating);

    audit_record_event(&node->data.audit_history_head, node->data.complaint_id,
                       "CLOSED", (actor != NULL) ? actor : "CUSTOMER", remarks);

    return SYS_OK;
}

void complaint_print_details(const Complaint *c) {
    if (c == NULL) {
        printf("  [!] Complaint is NULL.\n");
        return;
    }

    char created_str[32], updated_str[32];
    utils_format_datetime(c->created_at, created_str, sizeof(created_str));
    utils_format_datetime(c->updated_at, updated_str, sizeof(updated_str));

    const Customer *cust = customer_find_by_id(c->customer_id);
    const SupportAgent *agt = agent_find_by_id(c->assigned_agent_id);

    printf("  +--------------------------------------------------------------------------------------+\n");
    printf("  | Complaint ID   : %-66s |\n", c->complaint_id);
    printf("  | Category       : %-66s |\n", utils_category_to_string(c->category));
    printf("  | Priority       : %-66s |\n", utils_priority_to_string(c->priority));
    printf("  | Status         : %-66s |\n", utils_status_to_string(c->status));
    printf("  | Customer       : %s (%s) %-35s |\n",
           c->customer_id, (cust != NULL) ? cust->full_name : "Unknown", "");
    printf("  | Assigned Agent : %s (%s) %-35s |\n",
           (strlen(c->assigned_agent_id) > 0) ? c->assigned_agent_id : "Unassigned",
           (agt != NULL) ? agt->full_name : "None", "");
    printf("  | Registered At  : %-66s |\n", created_str);
    printf("  | Last Updated   : %-66s |\n", updated_str);
    printf("  | Description    : %-66.66s |\n", c->description);
    if (strlen(c->description) > 66) {
        printf("  |                  %-66.66s |\n", c->description + 66);
    }

    if (c->status == STATUS_RESOLVED || c->status == STATUS_CLOSED) {
        char res_time[32];
        utils_format_datetime(c->resolution.resolved_at, res_time, sizeof(res_time));
        printf("  |--------------------------------------------------------------------------------------|\n");
        printf("  | Resolution By  : %-66s |\n", c->resolution.resolved_by_agent_id);
        printf("  | Resolved At    : %-66s |\n", res_time);
        printf("  | Notes          : %-66.66s |\n", c->resolution.resolution_notes);
        if (c->status == STATUS_CLOSED) {
            printf("  | Rating         : %d / 5 Stars %-52s |\n", c->resolution.satisfaction_rating, "");
        }
    }
    printf("  +--------------------------------------------------------------------------------------+\n");
}

void complaint_display_all(const ComplaintList *list) {
    if (list == NULL || list->count == 0) {
        printf("\n  [i] No complaints recorded in the system.\n\n");
        return;
    }

    printf("\n  =========================================================================================================\n");
    printf("  %-10s | %-10s | %-14s | %-10s | %-12s | %-10s | %-24s\n",
           "ID", "Customer", "Category", "Priority", "Status", "Agent", "Description");
    printf("  ---------------------------------------------------------------------------------------------------------\n");

    ComplaintNode *curr = list->head;
    while (curr != NULL) {
        const Complaint *c = &curr->data;
        printf("  %-10s | %-10s | %-14s | %-10s | %-12s | %-10s | %-24.24s\n",
               c->complaint_id, c->customer_id,
               utils_category_to_string(c->category),
               utils_priority_to_string(c->priority),
               utils_status_to_string(c->status),
               (strlen(c->assigned_agent_id) > 0) ? c->assigned_agent_id : "Unassigned",
               c->description);
        curr = curr->next;
    }
    printf("  =========================================================================================================\n");
    printf("  Total Complaints: %zu\n\n", list->count);
}

void complaint_to_disk_record(const Complaint *c, ComplaintDiskRecord *rec) {
    if (c == NULL || rec == NULL) {
        return;
    }
    memset(rec, 0, sizeof(ComplaintDiskRecord));
    utils_safe_strcpy(rec->complaint_id, c->complaint_id, sizeof(rec->complaint_id));
    utils_safe_strcpy(rec->customer_id, c->customer_id, sizeof(rec->customer_id));
    rec->category = (int32_t)c->category;
    utils_safe_strcpy(rec->description, c->description, sizeof(rec->description));
    rec->priority = (int32_t)c->priority;
    rec->status = (int32_t)c->status;
    utils_safe_strcpy(rec->assigned_agent_id, c->assigned_agent_id, sizeof(rec->assigned_agent_id));
    rec->created_at = c->created_at;
    rec->updated_at = c->updated_at;
    rec->resolution = c->resolution;
}

SystemResult complaint_load_disk_record(ComplaintList *list, const ComplaintDiskRecord *rec) {
    if (list == NULL || rec == NULL) {
        return SYS_ERR_NULL_PTR;
    }

    ComplaintNode *node = (ComplaintNode *)malloc(sizeof(ComplaintNode));
    if (node == NULL) {
        return SYS_ERR_NO_MEMORY;
    }
    memset(node, 0, sizeof(ComplaintNode));

    utils_safe_strcpy(node->data.complaint_id, rec->complaint_id, sizeof(node->data.complaint_id));
    utils_safe_strcpy(node->data.customer_id, rec->customer_id, sizeof(node->data.customer_id));
    node->data.category = (ComplaintCategory)rec->category;
    utils_safe_strcpy(node->data.description, rec->description, sizeof(node->data.description));
    node->data.priority = (ComplaintPriority)rec->priority;
    node->data.status = (ComplaintStatus)rec->status;
    utils_safe_strcpy(node->data.assigned_agent_id, rec->assigned_agent_id, sizeof(node->data.assigned_agent_id));
    node->data.created_at = rec->created_at;
    node->data.updated_at = rec->updated_at;
    node->data.resolution = rec->resolution;
    node->data.audit_history_head = NULL;

    /* Append to doubly linked list */
    node->prev = list->tail;
    node->next = NULL;

    if (list->tail != NULL) {
        list->tail->next = node;
    } else {
        list->head = node;
    }
    list->tail = node;
    list->count++;

    /* Keep sequence counter aligned */
    unsigned int num = 0;
    if (sscanf(rec->complaint_id, "CMP-%04u", &num) == 1) {
        if (num >= list->next_id_counter) {
            list->next_id_counter = num + 1;
        }
    }

    /* Update agent workload if active ticket */
    if (node->data.status != STATUS_RESOLVED && node->data.status != STATUS_CLOSED) {
        if (strlen(node->data.assigned_agent_id) > 0) {
            agent_increment_workload(node->data.assigned_agent_id);
        }
    }

    return SYS_OK;
}
