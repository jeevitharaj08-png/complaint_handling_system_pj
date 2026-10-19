#include "report.h"
#include "agent.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

void report_summary_by_status(const ComplaintList *list) {
    if (list == NULL) {
        return;
    }

    int counts[STATUS_COUNT] = {0};
    ComplaintNode *curr = list->head;
    while (curr != NULL) {
        if (curr->data.status >= 0 && curr->data.status < STATUS_COUNT) {
            counts[curr->data.status]++;
        }
        curr = curr->next;
    }

    printf("\n  ========================================================================\n");
    printf("                  COMPLAINT SUMMARY REPORT BY STATUS                     \n");
    printf("  ========================================================================\n");
    printf("  %-16s | %-10s | %-12s | %s\n", "Status", "Count", "Percentage", "Distribution");
    printf("  ------------------------------------------------------------------------\n");

    for (int i = 0; i < STATUS_COUNT; i++) {
        double pct = (list->count > 0) ? ((double)counts[i] / (double)list->count) * 100.0 : 0.0;
        int bar_len = (int)(pct / 4.0); /* Scale bar to 25 chars */

        char bar[32];
        memset(bar, '#', bar_len);
        bar[bar_len] = '\0';

        printf("  %-16s | %-10d | %6.1f%%     | %-25s\n",
               utils_status_to_string((ComplaintStatus)i), counts[i], pct, bar);
    }
    printf("  ------------------------------------------------------------------------\n");
    printf("  Total Complaints Tracked: %zu\n", list->count);
    printf("  ========================================================================\n\n");
}

void report_summary_by_priority(const ComplaintList *list) {
    if (list == NULL) {
        return;
    }

    int counts[PRIORITY_COUNT] = {0};
    ComplaintNode *curr = list->head;
    while (curr != NULL) {
        if (curr->data.priority >= 0 && curr->data.priority < PRIORITY_COUNT) {
            counts[curr->data.priority]++;
        }
        curr = curr->next;
    }

    printf("\n  ========================================================================\n");
    printf("                 COMPLAINT SUMMARY REPORT BY PRIORITY                    \n");
    printf("  ========================================================================\n");
    printf("  %-16s | %-10s | %-12s | %s\n", "Priority", "Count", "Percentage", "Distribution");
    printf("  ------------------------------------------------------------------------\n");

    for (int i = 0; i < PRIORITY_COUNT; i++) {
        double pct = (list->count > 0) ? ((double)counts[i] / (double)list->count) * 100.0 : 0.0;
        int bar_len = (int)(pct / 4.0);

        char bar[32];
        memset(bar, '*', bar_len);
        bar[bar_len] = '\0';

        printf("  %-16s | %-10d | %6.1f%%     | %-25s\n",
               utils_priority_to_string((ComplaintPriority)i), counts[i], pct, bar);
    }
    printf("  ------------------------------------------------------------------------\n");
    printf("  Total Complaints Tracked: %zu\n", list->count);
    printf("  ========================================================================\n\n");
}

void report_summary_by_category(const ComplaintList *list) {
    if (list == NULL) {
        return;
    }

    int counts[CAT_COUNT] = {0};
    ComplaintNode *curr = list->head;
    while (curr != NULL) {
        if (curr->data.category >= 0 && curr->data.category < CAT_COUNT) {
            counts[curr->data.category]++;
        }
        curr = curr->next;
    }

    printf("\n  ========================================================================\n");
    printf("                 COMPLAINT SUMMARY REPORT BY CATEGORY                    \n");
    printf("  ========================================================================\n");
    printf("  %-18s | %-10s | %-12s\n", "Category", "Count", "Percentage");
    printf("  ------------------------------------------------------------------------\n");

    for (int i = 0; i < CAT_COUNT; i++) {
        double pct = (list->count > 0) ? ((double)counts[i] / (double)list->count) * 100.0 : 0.0;
        printf("  %-18s | %-10d | %6.1f%%\n",
               utils_category_to_string((ComplaintCategory)i), counts[i], pct);
    }
    printf("  ------------------------------------------------------------------------\n");
    printf("  Total Complaints Tracked: %zu\n", list->count);
    printf("  ========================================================================\n\n");
}

void report_agent_workload_and_performance(const ComplaintList *list) {
    if (list == NULL) {
        return;
    }

    size_t total_agents = agent_get_count();
    if (total_agents == 0) {
        printf("\n  [i] No support agents registered.\n\n");
        return;
    }

    printf("\n  ===================================================================================================\n");
    printf("                            SUPPORT AGENT WORKLOAD & PERFORMANCE REPORT                              \n");
    printf("  ===================================================================================================\n");
    printf("  %-10s | %-20s | %-14s | %-14s | %-14s | %s\n",
           "Agent ID", "Name", "Department", "Active Tickets", "Resolved Count", "Avg Rating");
    printf("  ---------------------------------------------------------------------------------------------------\n");

    for (size_t a = 0; a < total_agents; a++) {
        const SupportAgent *agt = agent_get_at(a);
        if (agt == NULL) continue;

        int resolved_count = 0;
        int rating_sum = 0;
        int rating_count = 0;

        ComplaintNode *curr = list->head;
        while (curr != NULL) {
            const Complaint *c = &curr->data;
            if (strcmp(c->assigned_agent_id, agt->agent_id) == 0 ||
                strcmp(c->resolution.resolved_by_agent_id, agt->agent_id) == 0) {

                if (c->status == STATUS_RESOLVED || c->status == STATUS_CLOSED) {
                    resolved_count++;
                }
                if (c->status == STATUS_CLOSED && c->resolution.satisfaction_rating > 0) {
                    rating_sum += c->resolution.satisfaction_rating;
                    rating_count++;
                }
            }
            curr = curr->next;
        }

        double avg_rating = (rating_count > 0) ? ((double)rating_sum / (double)rating_count) : 0.0;

        printf("  %-10s | %-20.20s | %-14.14s | %-14d | %-14d | %.2f / 5.0\n",
               agt->agent_id, agt->full_name, agt->department,
               agt->active_tickets_count, resolved_count, avg_rating);
    }
    printf("  ===================================================================================================\n\n");
}

void report_executive_dashboard(const ComplaintList *list) {
    if (list == NULL) {
        return;
    }

    int open_active = 0;
    int resolved_closed = 0;
    int escalated = 0;
    int critical = 0;
    int total_rating = 0;
    int rated_count = 0;

    ComplaintNode *curr = list->head;
    while (curr != NULL) {
        const Complaint *c = &curr->data;
        if (c->status == STATUS_RESOLVED || c->status == STATUS_CLOSED) {
            resolved_closed++;
        } else {
            open_active++;
        }
        if (c->status == STATUS_ESCALATED) {
            escalated++;
        }
        if (c->priority == PRIORITY_CRITICAL) {
            critical++;
        }
        if (c->status == STATUS_CLOSED && c->resolution.satisfaction_rating > 0) {
            total_rating += c->resolution.satisfaction_rating;
            rated_count++;
        }
        curr = curr->next;
    }

    double res_rate = (list->count > 0) ? ((double)resolved_closed / (double)list->count) * 100.0 : 0.0;
    double avg_rating = (rated_count > 0) ? ((double)total_rating / (double)rated_count) : 0.0;

    printf("\n  +=====================================================================+\n");
    printf("  |                    SYSTEM EXECUTIVE DASHBOARD                       |\n");
    printf("  +=====================================================================+\n");
    printf("  | Total Complaints Registered : %-37zu |\n", list->count);
    printf("  | Active / Open Complaints    : %-37d |\n", open_active);
    printf("  | Resolved / Closed Complaints: %-37d |\n", resolved_closed);
    printf("  | Escalated Complaints        : %-37d |\n", escalated);
    printf("  | Critical Priority Tickets   : %-37d |\n", critical);
    printf("  | Overall Resolution Rate     : %5.1f%% %-31s |\n", res_rate, "");
    printf("  | Average Customer Rating     : %3.1f / 5.0 (%d reviews) %-21s |\n", avg_rating, rated_count, "");
    printf("  +=====================================================================+\n\n");
}
