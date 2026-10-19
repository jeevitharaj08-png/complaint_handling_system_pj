#include "ui.h"
#include "customer.h"
#include "agent.h"
#include "complaint.h"
#include "search.h"
#include "report.h"
#include "storage.h"
#include "audit.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SEARCH_RESULTS 256

/* ========================================================================= */
/* Helper Input Pickers                                                      */
/* ========================================================================= */

static ComplaintCategory prompt_category(void) {
    printf("\n  Select Category:\n");
    for (int i = 0; i < CAT_COUNT; i++) {
        printf("    %d. %s\n", i + 1, utils_category_to_string((ComplaintCategory)i));
    }
    printf("  Choice [1-%d]: ", CAT_COUNT);
    int choice = 1;
    utils_read_int(1, CAT_COUNT, &choice);
    return (ComplaintCategory)(choice - 1);
}

static ComplaintPriority prompt_priority(void) {
    printf("\n  Select Priority:\n");
    for (int i = 0; i < PRIORITY_COUNT; i++) {
        printf("    %d. %s\n", i + 1, utils_priority_to_string((ComplaintPriority)i));
    }
    printf("  Choice [1-%d]: ", PRIORITY_COUNT);
    int choice = 2;
    utils_read_int(1, PRIORITY_COUNT, &choice);
    return (ComplaintPriority)(choice - 1);
}

static ComplaintStatus prompt_status(void) {
    printf("\n  Select Status:\n");
    for (int i = 0; i < STATUS_COUNT; i++) {
        printf("    %d. %s\n", i + 1, utils_status_to_string((ComplaintStatus)i));
    }
    printf("  Choice [1-%d]: ", STATUS_COUNT);
    int choice = 1;
    utils_read_int(1, STATUS_COUNT, &choice);
    return (ComplaintStatus)(choice - 1);
}

/* ========================================================================= */
/* Complaint Sub-Workflows                                                   */
/* ========================================================================= */

static void ui_register_complaint(ComplaintList *list) {
    printf("\n  --- REGISTER NEW COMPLAINT ---\n");

    char cust_id[MAX_ID_LEN];
    printf("  Enter Customer ID (or type 'NEW' to register customer first): ");
    utils_read_line(cust_id, sizeof(cust_id));

    if (strcmp(cust_id, "NEW") == 0 || strcmp(cust_id, "new") == 0) {
        char name[MAX_NAME_LEN], email[MAX_EMAIL_LEN], phone[MAX_PHONE_LEN], addr[MAX_ADDR_LEN];
        printf("\n  Register New Customer:\n");
        printf("    Full Name    : "); utils_read_line(name, sizeof(name));
        printf("    Phone Number : "); utils_read_line(phone, sizeof(phone));
        printf("    Email Address: "); utils_read_line(email, sizeof(email));
        printf("    Address      : "); utils_read_line(addr, sizeof(addr));

        SystemResult cr = customer_add(name, email, phone, addr, cust_id);
        if (cr != SYS_OK) {
            printf("  [!] Failed to register customer: %s\n", utils_result_to_string(cr));
            utils_pause_prompt();
            return;
        }
        printf("  [+] Registered Customer ID: %s\n", cust_id);
    } else {
        const Customer *cust = customer_find_by_id(cust_id);
        if (cust == NULL) {
            printf("  [!] Customer ID '%s' not found in records.\n", cust_id);
            printf("      Please check registered customers or create a customer first.\n");
            utils_pause_prompt();
            return;
        }
        printf("  [+] Verified Customer: %s (%s)\n", cust->full_name, cust->phone);
    }

    ComplaintCategory cat = prompt_category();
    ComplaintPriority pri = prompt_priority();

    char desc[MAX_DESC_LEN];
    printf("\n  Enter Detailed Description:\n  > ");
    utils_read_line(desc, sizeof(desc));

    char new_id[MAX_ID_LEN];
    SystemResult res = complaint_register(list, cust_id, cat, desc, pri, "SYSTEM_CONSOLE", new_id);
    if (res == SYS_OK) {
        printf("\n  [SUCCESS] Complaint registered successfully! Ticket ID: %s\n", new_id);

        printf("\n  Would you like to auto-assign this ticket to the least-loaded agent now?\n");
        printf("    1. Yes (Auto-dispatch to specialist/least-busy)\n");
        printf("    2. No  (Keep in OPEN queue)\n");
        printf("  Choice [1-2]: ");
        int assign_choice = 1;
        utils_read_int(1, 2, &assign_choice);
        if (assign_choice == 1) {
            SystemResult a_res = complaint_auto_assign(list, new_id, "AUTO-DISPATCH");
            if (a_res == SYS_OK) {
                ComplaintNode *cn = complaint_find_by_id(list, new_id);
                if (cn != NULL) {
                    SupportAgent *agt = agent_find_by_id(cn->data.assigned_agent_id);
                    printf("  [+] Auto-assigned to: %s (%s - %s)\n",
                           (agt != NULL) ? agt->full_name : cn->data.assigned_agent_id,
                           cn->data.assigned_agent_id,
                           (agt != NULL) ? agt->department : "General");
                }
            } else {
                printf("  [!] Auto-assignment could not find an available agent.\n");
            }
        }
    } else {
        printf("  [!] Registration failed: %s\n", utils_result_to_string(res));
    }
    utils_pause_prompt();
}

static void ui_assign_complaint(ComplaintList *list) {
    printf("\n  --- ASSIGN / REASSIGN COMPLAINT ---\n");
    char id[MAX_ID_LEN];
    printf("  Enter Complaint ID (e.g. CMP-1001): ");
    utils_read_line(id, sizeof(id));

    ComplaintNode *node = complaint_find_by_id(list, id);
    if (node == NULL) {
        printf("  [!] Complaint ID '%s' not found.\n", id);
        utils_pause_prompt();
        return;
    }

    printf("  Current Status : %s | Priority: %s | Category: %s\n",
           utils_status_to_string(node->data.status),
           utils_priority_to_string(node->data.priority),
           utils_category_to_string(node->data.category));
    printf("  Current Agent  : %s\n",
           (strlen(node->data.assigned_agent_id) > 0) ? node->data.assigned_agent_id : "None (Unassigned)");

    printf("\n  Assignment Options:\n");
    printf("    1. Auto-Assign (Select available agent with fewest active complaints)\n");
    printf("    2. Manual Selection from Staff Directory\n");
    printf("  Choice [1-2]: ");
    int choice = 1;
    utils_read_int(1, 2, &choice);

    if (choice == 1) {
        SystemResult res = complaint_auto_assign(list, id, "SUPERVISOR");
        if (res == SYS_OK) {
            SupportAgent *agt = agent_find_by_id(node->data.assigned_agent_id);
            printf("  [SUCCESS] Auto-assigned to %s (%s) | Department: %s\n",
                   (agt != NULL) ? agt->full_name : node->data.assigned_agent_id,
                   node->data.assigned_agent_id,
                   (agt != NULL) ? agt->department : "General");
        } else {
            printf("  [!] Auto-assign failed: %s\n", utils_result_to_string(res));
        }
    } else {
        agent_display_all();
        char agt_id[MAX_ID_LEN];
        printf("  Enter Support Agent ID to assign: ");
        utils_read_line(agt_id, sizeof(agt_id));

        SystemResult res = complaint_assign_agent(list, id, agt_id, "SUPERVISOR");
        if (res == SYS_OK) {
            printf("  [SUCCESS] Complaint %s successfully assigned to Agent %s.\n", id, agt_id);
        } else {
            printf("  [!] Assignment failed: %s\n", utils_result_to_string(res));
        }
    }
    utils_pause_prompt();
}

static void ui_mark_in_progress(ComplaintList *list) {
    printf("\n  --- MARK COMPLAINT IN PROGRESS ---\n");
    char id[MAX_ID_LEN];
    printf("  Enter Complaint ID: ");
    utils_read_line(id, sizeof(id));

    char notes[MAX_NOTES_LEN];
    printf("  Enter work progress notes / updates: ");
    utils_read_line(notes, sizeof(notes));

    SystemResult res = complaint_mark_in_progress(list, id, "AGENT", notes);
    if (res == SYS_OK) {
        printf("  [SUCCESS] Status updated to IN_PROGRESS for %s.\n", id);
    } else {
        printf("  [!] Failed: %s\n", utils_result_to_string(res));
    }
    utils_pause_prompt();
}

static void ui_change_priority(ComplaintList *list) {
    printf("\n  --- CHANGE COMPLAINT PRIORITY ---\n");
    char id[MAX_ID_LEN];
    printf("  Enter Complaint ID: ");
    utils_read_line(id, sizeof(id));

    ComplaintNode *node = complaint_find_by_id(list, id);
    if (node == NULL) {
        printf("  [!] Complaint ID '%s' not found.\n", id);
        utils_pause_prompt();
        return;
    }

    printf("  Current Priority: %s\n", utils_priority_to_string(node->data.priority));
    ComplaintPriority new_pri = prompt_priority();

    char reason[MAX_NOTES_LEN];
    printf("  Enter justification / reason for priority change: ");
    utils_read_line(reason, sizeof(reason));

    SystemResult res = complaint_change_priority(list, id, new_pri, "SUPERVISOR", reason);
    if (res == SYS_OK) {
        printf("  [SUCCESS] Priority updated to %s for %s.\n", utils_priority_to_string(new_pri), id);
    } else {
        printf("  [!] Failed: %s\n", utils_result_to_string(res));
    }
    utils_pause_prompt();
}

static void ui_escalate_manual(ComplaintList *list) {
    printf("\n  --- ESCALATE COMPLAINT (MANUAL) ---\n");
    char id[MAX_ID_LEN];
    printf("  Enter Complaint ID: ");
    utils_read_line(id, sizeof(id));

    char reason[MAX_NOTES_LEN];
    printf("  Enter reason for escalation: ");
    utils_read_line(reason, sizeof(reason));

    SystemResult res = complaint_escalate_manual(list, id, "SUPERVISOR", reason);
    if (res == SYS_OK) {
        printf("  [SUCCESS] Complaint %s has been escalated to CRITICAL priority!\n", id);
    } else {
        printf("  [!] Escalation failed: %s\n", utils_result_to_string(res));
    }
    utils_pause_prompt();
}

static void ui_auto_escalate_scan(ComplaintList *list) {
    printf("\n  --- AUTO-ESCALATION SCANNER ---\n");
    printf("  Threshold Rule: CRITICAL tickets open longer than %d hours are auto-escalated.\n",
           list->critical_escalation_hours);
    printf("  Scanning active tickets...\n");

    size_t count = complaint_auto_escalate_critical(list, "SLA-MONITOR");
    if (count > 0) {
        printf("  [!] SLA Warning: %zu critical complaint(s) breached threshold and were escalated!\n", count);
    } else {
        printf("  [+] All critical complaints are currently within acceptable SLA limits.\n");
    }
    utils_pause_prompt();
}

static void ui_configure_escalation_sla(ComplaintList *list) {
    printf("\n  --- CONFIGURE ESCALATION SLA THRESHOLD ---\n");
    printf("  Current Threshold: %d hour(s)\n", list->critical_escalation_hours);
    printf("  Enter new threshold in hours [1 - 720]: ");
    int new_val = list->critical_escalation_hours;
    utils_read_int(1, 720, &new_val);
    list->critical_escalation_hours = new_val;
    printf("  [SUCCESS] Escalation SLA threshold updated to %d hour(s).\n", list->critical_escalation_hours);
    utils_pause_prompt();
}

static void ui_resolve_complaint(ComplaintList *list) {
    printf("\n  --- RESOLVE COMPLAINT ---\n");
    char id[MAX_ID_LEN];
    printf("  Enter Complaint ID: ");
    utils_read_line(id, sizeof(id));

    ComplaintNode *node = complaint_find_by_id(list, id);
    if (node == NULL) {
        printf("  [!] Complaint ID '%s' not found.\n", id);
        utils_pause_prompt();
        return;
    }

    char agent_id[MAX_ID_LEN];
    printf("  Enter Resolver Agent ID [%s]: ",
           (strlen(node->data.assigned_agent_id) > 0) ? node->data.assigned_agent_id : "AGT-01");
    utils_read_line(agent_id, sizeof(agent_id));
    if (strlen(agent_id) == 0) {
        utils_safe_strcpy(agent_id,
                         (strlen(node->data.assigned_agent_id) > 0) ? node->data.assigned_agent_id : "AGT-01",
                         sizeof(agent_id));
    }

    char notes[MAX_NOTES_LEN];
    printf("  Enter Detailed Resolution Notes:\n  > ");
    utils_read_line(notes, sizeof(notes));

    SystemResult res = complaint_resolve(list, id, agent_id, notes);
    if (res == SYS_OK) {
        printf("  [SUCCESS] Complaint %s marked as RESOLVED.\n", id);
        printf("  (The customer may now confirm closure and provide a satisfaction rating)\n");
    } else {
        printf("  [!] Failed to resolve complaint: %s\n", utils_result_to_string(res));
    }
    utils_pause_prompt();
}

static void ui_close_complaint(ComplaintList *list) {
    printf("\n  --- CLOSE COMPLAINT & CUSTOMER FEEDBACK ---\n");
    char id[MAX_ID_LEN];
    printf("  Enter Complaint ID: ");
    utils_read_line(id, sizeof(id));

    ComplaintNode *node = complaint_find_by_id(list, id);
    if (node == NULL) {
        printf("  [!] Complaint ID '%s' not found.\n", id);
        utils_pause_prompt();
        return;
    }

    if (node->data.status != STATUS_RESOLVED) {
        printf("  [!] Complaint cannot be closed directly: current status is '%s'.\n",
               utils_status_to_string(node->data.status));
        printf("      Complaints must be in 'RESOLVED' status before customer closure.\n");
        utils_pause_prompt();
        return;
    }

    printf("  Resolution Notes: %s\n", node->data.resolution.resolution_notes);
    printf("  Rate your satisfaction (1 = Very Dissatisfied, 5 = Very Satisfied): ");
    int rating = 5;
    utils_read_int(1, 5, &rating);

    SystemResult res = complaint_close(list, id, rating, "CUSTOMER");
    if (res == SYS_OK) {
        printf("  [SUCCESS] Complaint %s has been CLOSED. Thank you for your feedback (%d/5 stars)!\n", id, rating);
    } else {
        printf("  [!] Closure failed: %s\n", utils_result_to_string(res));
    }
    utils_pause_prompt();
}

static void ui_view_single_complaint(const ComplaintList *list) {
    printf("\n  --- VIEW COMPLAINT DETAILS & TIMELINE ---\n");
    char id[MAX_ID_LEN];
    printf("  Enter Complaint ID: ");
    utils_read_line(id, sizeof(id));

    ComplaintNode *node = complaint_find_by_id(list, id);
    if (node == NULL) {
        printf("  [!] Complaint ID '%s' not found.\n", id);
        utils_pause_prompt();
        return;
    }

    complaint_print_details(&node->data);

    printf("\n  View full chronological audit timeline for this ticket?\n");
    printf("    1. Yes\n    2. No\n  Choice [1-2]: ");
    int choice = 1;
    utils_read_int(1, 2, &choice);
    if (choice == 1) {
        audit_print_history(node->data.audit_history_head, node->data.complaint_id);
    }
    utils_pause_prompt();
}

/* ========================================================================= */
/* Customer Sub-Workflows                                                    */
/* ========================================================================= */

static void ui_customer_menu(void) {
    while (1) {
        printf("\n  ========================================\n");
        printf("            CUSTOMER MANAGEMENT           \n");
        printf("  ========================================\n");
        printf("    1. Register New Customer\n");
        printf("    2. Search Customer by ID\n");
        printf("    3. Search Customer by Phone Number\n");
        printf("    4. List All Registered Customers\n");
        printf("    0. Back to Main Menu\n");
        printf("  ----------------------------------------\n");
        printf("  Enter choice [0-4]: ");
        int ch = 0;
        utils_read_int(0, 4, &ch);

        if (ch == 0) break;

        switch (ch) {
            case 1: {
                char name[MAX_NAME_LEN], email[MAX_EMAIL_LEN], phone[MAX_PHONE_LEN], addr[MAX_ADDR_LEN], new_id[MAX_ID_LEN];
                printf("\n  Full Name    : "); utils_read_line(name, sizeof(name));
                printf("  Phone Number : "); utils_read_line(phone, sizeof(phone));
                printf("  Email Address: "); utils_read_line(email, sizeof(email));
                printf("  Address      : "); utils_read_line(addr, sizeof(addr));

                SystemResult res = customer_add(name, email, phone, addr, new_id);
                if (res == SYS_OK) {
                    printf("  [SUCCESS] Customer created successfully! ID: %s\n", new_id);
                } else {
                    printf("  [!] Failed: %s\n", utils_result_to_string(res));
                }
                utils_pause_prompt();
                break;
            }
            case 2: {
                char id[MAX_ID_LEN];
                printf("\n  Enter Customer ID: ");
                utils_read_line(id, sizeof(id));
                const Customer *c = customer_find_by_id(id);
                if (c != NULL) {
                    customer_print_details(c);
                } else {
                    printf("  [!] Customer ID not found.\n");
                }
                utils_pause_prompt();
                break;
            }
            case 3: {
                char phone[MAX_PHONE_LEN];
                printf("\n  Enter Phone Number: ");
                utils_read_line(phone, sizeof(phone));
                const Customer *c = customer_find_by_phone(phone);
                if (c != NULL) {
                    customer_print_details(c);
                } else {
                    printf("  [!] No customer found with phone: %s\n", phone);
                }
                utils_pause_prompt();
                break;
            }
            case 4:
                customer_display_all();
                utils_pause_prompt();
                break;
        }
    }
}

/* ========================================================================= */
/* Support Agent Sub-Workflows                                               */
/* ========================================================================= */

static void ui_agent_menu(void) {
    while (1) {
        printf("\n  ========================================\n");
        printf("          SUPPORT AGENT MANAGEMENT        \n");
        printf("  ========================================\n");
        printf("    1. Register New Support Agent\n");
        printf("    2. View All Agents & Live Workloads\n");
        printf("    0. Back to Main Menu\n");
        printf("  ----------------------------------------\n");
        printf("  Enter choice [0-2]: ");
        int ch = 0;
        utils_read_int(0, 2, &ch);

        if (ch == 0) break;

        switch (ch) {
            case 1: {
                char name[MAX_NAME_LEN], dept[MAX_CATEGORY_LEN], new_id[MAX_ID_LEN];
                printf("\n  Agent Name : "); utils_read_line(name, sizeof(name));
                printf("  Department (e.g. ACADEMIC, HOSTEL, INFRASTRUCTURE): ");
                utils_read_line(dept, sizeof(dept));

                SystemResult res = agent_register(name, dept, new_id);
                if (res == SYS_OK) {
                    printf("  [SUCCESS] Agent registered! ID: %s\n", new_id);
                } else {
                    printf("  [!] Registration failed: %s\n", utils_result_to_string(res));
                }
                utils_pause_prompt();
                break;
            }
            case 2:
                agent_display_all();
                utils_pause_prompt();
                break;
        }
    }
}

/* ========================================================================= */
/* Search Sub-Workflows                                                      */
/* ========================================================================= */

static void ui_search_menu(const ComplaintList *list) {
    const Complaint *results[MAX_SEARCH_RESULTS];

    while (1) {
        printf("\n  ========================================\n");
        printf("            SEARCH & QUERY ENGINE         \n");
        printf("  ========================================\n");
        printf("    1. Search by Complaint ID\n");
        printf("    2. Search by Customer ID\n");
        printf("    3. Search by Status\n");
        printf("    4. Search by Priority\n");
        printf("    5. Search by Assigned Agent ID\n");
        printf("    6. Search by Keyword in Description\n");
        printf("    0. Back to Main Menu\n");
        printf("  ----------------------------------------\n");
        printf("  Enter choice [0-6]: ");
        int ch = 0;
        utils_read_int(0, 6, &ch);

        if (ch == 0) break;

        size_t count = 0;
        switch (ch) {
            case 1: {
                char id[MAX_ID_LEN];
                printf("\n  Enter Complaint ID: "); utils_read_line(id, sizeof(id));
                count = search_by_id(list, id, results, MAX_SEARCH_RESULTS);
                search_display_results(results, count);
                break;
            }
            case 2: {
                char cid[MAX_ID_LEN];
                printf("\n  Enter Customer ID: "); utils_read_line(cid, sizeof(cid));
                count = search_by_customer(list, cid, results, MAX_SEARCH_RESULTS);
                search_display_results(results, count);
                break;
            }
            case 3: {
                ComplaintStatus st = prompt_status();
                count = search_by_status(list, st, results, MAX_SEARCH_RESULTS);
                search_display_results(results, count);
                break;
            }
            case 4: {
                ComplaintPriority pr = prompt_priority();
                count = search_by_priority(list, pr, results, MAX_SEARCH_RESULTS);
                search_display_results(results, count);
                break;
            }
            case 5: {
                char aid[MAX_ID_LEN];
                printf("\n  Enter Agent ID: "); utils_read_line(aid, sizeof(aid));
                count = search_by_agent(list, aid, results, MAX_SEARCH_RESULTS);
                search_display_results(results, count);
                break;
            }
            case 6: {
                char kw[64];
                printf("\n  Enter Keyword to search: "); utils_read_line(kw, sizeof(kw));
                count = search_by_keyword(list, kw, results, MAX_SEARCH_RESULTS);
                search_display_results(results, count);
                break;
            }
        }
        utils_pause_prompt();
    }
}

/* ========================================================================= */
/* Reporting Sub-Workflows                                                   */
/* ========================================================================= */

static void ui_reports_menu(const ComplaintList *list) {
    while (1) {
        printf("\n  ========================================\n");
        printf("         SUMMARY REPORTS & ANALYTICS      \n");
        printf("  ========================================\n");
        printf("    1. Executive KPI Dashboard\n");
        printf("    2. Summary Breakdown by Status\n");
        printf("    3. Summary Breakdown by Priority\n");
        printf("    4. Summary Breakdown by Category\n");
        printf("    5. Support Agent Workload & Performance\n");
        printf("    0. Back to Main Menu\n");
        printf("  ----------------------------------------\n");
        printf("  Enter choice [0-5]: ");
        int ch = 0;
        utils_read_int(0, 5, &ch);

        if (ch == 0) break;

        switch (ch) {
            case 1: report_executive_dashboard(list); break;
            case 2: report_summary_by_status(list); break;
            case 3: report_summary_by_priority(list); break;
            case 4: report_summary_by_category(list); break;
            case 5: report_agent_workload_and_performance(list); break;
        }
        utils_pause_prompt();
    }
}

/* ========================================================================= */
/* Main Menu Orchestration                                                   */
/* ========================================================================= */

void ui_run_main_menu(ComplaintList *complaints) {
    while (1) {
        printf("\n=================================================================\n");
        printf("           COLLEGE COMPLAINT HANDLING SYSTEM (C11)               \n");
        printf("=================================================================\n");
        printf("  1.  Register New Complaint\n");
        printf("  2.  Assign / Reassign Complaint (Manual or Auto Least-Loaded)\n");
        printf("  3.  Advance Status to IN PROGRESS\n");
        printf("  4.  Change Complaint Priority (with logged justification)\n");
        printf("  5.  Escalate Complaint (Manual Escalation)\n");
        printf("  6.  Run Automatic SLA Escalation Scanner (%d hrs threshold)\n", complaints->critical_escalation_hours);
        printf("  7.  Configure Escalation SLA Threshold\n");
        printf("  8.  Resolve Complaint (Enter resolution notes & resolver)\n");
        printf("  9.  Close Complaint & Capture Customer Feedback (1-5 Stars)\n");
        printf("  10. View Single Complaint Card & Audit Timeline\n");
        printf("  11. View All Complaints (Tabular List)\n");
        printf("  ---------------------------------------------------------------\n");
        printf("  12. Customer Management Menu\n");
        printf("  13. Support Agent Management Menu\n");
        printf("  14. Search & Multi-Criteria Queries\n");
        printf("  15. System Audit Journal (audit.log)\n");
        printf("  16. Summary Reports & Analytics Dashboard\n");
        printf("  17. Save All Records to Persistent Storage\n");
        printf("  0.  Exit System (Auto-Save & Safe Cleanup)\n");
        printf("=================================================================\n");
        printf("  Enter command [0-17]: ");

        int choice = 0;
        utils_read_int(0, 17, &choice);

        if (choice == 0) {
            printf("\n  Saving system state to persistent binary storage...\n");
            SystemResult sres = storage_save_all(complaints);
            if (sres == SYS_OK) {
                printf("  [SUCCESS] All records safely committed to disk.\n");
            } else {
                printf("  [!] Error saving state: %s\n", utils_result_to_string(sres));
            }
            printf("  Exiting Complaint Handling System. Goodbye!\n\n");
            break;
        }

        switch (choice) {
            case 1:  ui_register_complaint(complaints); break;
            case 2:  ui_assign_complaint(complaints); break;
            case 3:  ui_mark_in_progress(complaints); break;
            case 4:  ui_change_priority(complaints); break;
            case 5:  ui_escalate_manual(complaints); break;
            case 6:  ui_auto_escalate_scan(complaints); break;
            case 7:  ui_configure_escalation_sla(complaints); break;
            case 8:  ui_resolve_complaint(complaints); break;
            case 9:  ui_close_complaint(complaints); break;
            case 10: ui_view_single_complaint(complaints); break;
            case 11: complaint_display_all(complaints); utils_pause_prompt(); break;
            case 12: ui_customer_menu(); break;
            case 13: ui_agent_menu(); break;
            case 14: ui_search_menu(complaints); break;
            case 15: audit_display_global_log(50); utils_pause_prompt(); break;
            case 16: ui_reports_menu(complaints); break;
            case 17: {
                SystemResult sr = storage_save_all(complaints);
                if (sr == SYS_OK) {
                    printf("\n  [SUCCESS] State successfully saved to disk!\n");
                } else {
                    printf("\n  [!] Failed to save: %s\n", utils_result_to_string(sr));
                }
                utils_pause_prompt();
                break;
            }
        }
    }
}
