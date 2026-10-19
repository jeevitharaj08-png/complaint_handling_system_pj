#include "search.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

size_t search_by_id(const ComplaintList *list,
                    const char *id,
                    const Complaint **out_results,
                    size_t max_results) {
    if (list == NULL || id == NULL || out_results == NULL || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    ComplaintNode *curr = list->head;
    while (curr != NULL && count < max_results) {
        if (strcmp(curr->data.complaint_id, id) == 0) {
            out_results[count++] = &curr->data;
            break;
        }
        curr = curr->next;
    }
    return count;
}

size_t search_by_customer(const ComplaintList *list,
                          const char *customer_id,
                          const Complaint **out_results,
                          size_t max_results) {
    if (list == NULL || customer_id == NULL || out_results == NULL || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    ComplaintNode *curr = list->head;
    while (curr != NULL && count < max_results) {
        if (strcmp(curr->data.customer_id, customer_id) == 0) {
            out_results[count++] = &curr->data;
        }
        curr = curr->next;
    }
    return count;
}

size_t search_by_status(const ComplaintList *list,
                        ComplaintStatus status,
                        const Complaint **out_results,
                        size_t max_results) {
    if (list == NULL || out_results == NULL || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    ComplaintNode *curr = list->head;
    while (curr != NULL && count < max_results) {
        if (curr->data.status == status) {
            out_results[count++] = &curr->data;
        }
        curr = curr->next;
    }
    return count;
}

size_t search_by_priority(const ComplaintList *list,
                          ComplaintPriority priority,
                          const Complaint **out_results,
                          size_t max_results) {
    if (list == NULL || out_results == NULL || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    ComplaintNode *curr = list->head;
    while (curr != NULL && count < max_results) {
        if (curr->data.priority == priority) {
            out_results[count++] = &curr->data;
        }
        curr = curr->next;
    }
    return count;
}

size_t search_by_agent(const ComplaintList *list,
                       const char *agent_id,
                       const Complaint **out_results,
                       size_t max_results) {
    if (list == NULL || agent_id == NULL || out_results == NULL || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    ComplaintNode *curr = list->head;
    while (curr != NULL && count < max_results) {
        if (strcmp(curr->data.assigned_agent_id, agent_id) == 0) {
            out_results[count++] = &curr->data;
        }
        curr = curr->next;
    }
    return count;
}

size_t search_by_keyword(const ComplaintList *list,
                         const char *keyword,
                         const Complaint **out_results,
                         size_t max_results) {
    if (list == NULL || keyword == NULL || out_results == NULL || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    ComplaintNode *curr = list->head;
    while (curr != NULL && count < max_results) {
        if (strstr(curr->data.description, keyword) != NULL) {
            out_results[count++] = &curr->data;
        }
        curr = curr->next;
    }
    return count;
}

void search_display_results(const Complaint **results, size_t count) {
    if (results == NULL || count == 0) {
        printf("\n  [i] No matching complaints found for the specified query.\n\n");
        return;
    }

    printf("\n  =========================================================================================================\n");
    printf("  Found %zu Matching Complaint(s):\n", count);
    printf("  ---------------------------------------------------------------------------------------------------------\n");
    printf("  %-10s | %-10s | %-14s | %-10s | %-12s | %-10s | %-24s\n",
           "ID", "Customer", "Category", "Priority", "Status", "Agent", "Description");
    printf("  ---------------------------------------------------------------------------------------------------------\n");

    for (size_t i = 0; i < count; i++) {
        const Complaint *c = results[i];
        printf("  %-10s | %-10s | %-14s | %-10s | %-12s | %-10s | %-24.24s\n",
               c->complaint_id, c->customer_id,
               utils_category_to_string(c->category),
               utils_priority_to_string(c->priority),
               utils_status_to_string(c->status),
               (strlen(c->assigned_agent_id) > 0) ? c->assigned_agent_id : "Unassigned",
               c->description);
    }
    printf("  =========================================================================================================\n\n");
}
