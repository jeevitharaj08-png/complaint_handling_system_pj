#ifndef REPORT_H
#define REPORT_H

#include "complaint.h"

/**
 * @brief Generates and prints summary report grouped by complaint status.
 */
void report_summary_by_status(const ComplaintList *list);

/**
 * @brief Generates and prints summary report grouped by priority level.
 */
void report_summary_by_priority(const ComplaintList *list);

/**
 * @brief Generates and prints summary report grouped by complaint category.
 */
void report_summary_by_category(const ComplaintList *list);

/**
 * @brief Generates and prints agent workload and performance metrics.
 */
void report_agent_workload_and_performance(const ComplaintList *list);

/**
 * @brief Displays an executive dashboard summarizing all operational KPIs.
 */
void report_executive_dashboard(const ComplaintList *list);

#endif /* REPORT_H */
