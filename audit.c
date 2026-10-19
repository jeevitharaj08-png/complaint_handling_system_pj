#include "audit.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define ENSURE_DATA_DIR() _mkdir(DATA_DIR)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define ENSURE_DATA_DIR() mkdir(DATA_DIR, 0755)
#endif

void audit_record_event(AuditNode **history_head,
                        const char *complaint_id,
                        const char *action,
                        const char *actor,
                        const char *remarks) {
    DateTime now = utils_get_current_time();
    char time_str[32];
    utils_format_datetime(now, time_str, sizeof(time_str));

    const char *safe_cid = (complaint_id != NULL) ? complaint_id : "SYS-GLOBAL";
    const char *safe_act = (action != NULL) ? action : "EVENT";
    const char *safe_usr = (actor != NULL) ? actor : "SYSTEM";
    const char *safe_rem = (remarks != NULL) ? remarks : "N/A";

    /* 1. Append to persistent audit log file */
    ENSURE_DATA_DIR();
    FILE *fp = fopen(FILE_AUDIT_LOG, "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] | %-10s | %-12s | %-16s | %s\n",
                time_str, safe_cid, safe_act, safe_usr, safe_rem);
        fflush(fp);
        fclose(fp);
    }

    /* 2. If an in-memory history head pointer is provided, append to the linked list */
    if (history_head != NULL) {
        AuditNode *node = (AuditNode *)malloc(sizeof(AuditNode));
        if (node != NULL) {
            memset(node, 0, sizeof(AuditNode));
            node->timestamp = now;
            utils_safe_strcpy(node->complaint_id, safe_cid, sizeof(node->complaint_id));
            utils_safe_strcpy(node->action, safe_act, sizeof(node->action));
            utils_safe_strcpy(node->actor, safe_usr, sizeof(node->actor));
            utils_safe_strcpy(node->remarks, safe_rem, sizeof(node->remarks));
            node->next = NULL;

            if (*history_head == NULL) {
                *history_head = node;
            } else {
                AuditNode *curr = *history_head;
                while (curr->next != NULL) {
                    curr = curr->next;
                }
                curr->next = node;
            }
        }
    }
}

void audit_free_chain(AuditNode *head) {
    AuditNode *curr = head;
    while (curr != NULL) {
        AuditNode *next = curr->next;
        free(curr);
        curr = next;
    }
}

void audit_print_history(const AuditNode *head, const char *complaint_id) {
    printf("\n  ========================================================================================\n");
    printf("  Audit Trail & Lifecycle History for Complaint: %s\n",
           (complaint_id != NULL) ? complaint_id : "N/A");
    printf("  ----------------------------------------------------------------------------------------\n");
    printf("  %-20s | %-12s | %-18s | %s\n", "Timestamp", "Action", "Actor", "Remarks");
    printf("  ----------------------------------------------------------------------------------------\n");

    if (head == NULL) {
        printf("  (No recorded audit entries in memory)\n");
    } else {
        const AuditNode *curr = head;
        while (curr != NULL) {
            char time_str[32];
            utils_format_datetime(curr->timestamp, time_str, sizeof(time_str));
            printf("  %-20s | %-12s | %-18.18s | %s\n",
                   time_str, curr->action, curr->actor, curr->remarks);
            curr = curr->next;
        }
    }
    printf("  ========================================================================================\n\n");
}

void audit_display_global_log(size_t max_lines) {
    FILE *fp = fopen(FILE_AUDIT_LOG, "r");
    if (fp == NULL) {
        printf("\n  [i] Audit log file (%s) is empty or not yet created.\n\n", FILE_AUDIT_LOG);
        return;
    }

    printf("\n  ========================================================================================\n");
    printf("  System-Wide Audit Journal (%s)\n", FILE_AUDIT_LOG);
    printf("  ----------------------------------------------------------------------------------------\n");

    char line[512];
    size_t count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        printf("  %s", line);
        count++;
        if (max_lines > 0 && count >= max_lines) {
            printf("  ... (output truncated at %zu lines)\n", max_lines);
            break;
        }
    }

    fclose(fp);
    printf("  ========================================================================================\n\n");
}
