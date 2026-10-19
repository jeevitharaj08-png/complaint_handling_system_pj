#ifndef SEARCH_H
#define SEARCH_H

#include "complaint.h"
#include <stddef.h>

/**
 * @brief Searches complaints by exact Complaint ID.
 */
size_t search_by_id(const ComplaintList *list,
                    const char *id,
                    const Complaint **out_results,
                    size_t max_results);

/**
 * @brief Searches complaints by Customer ID.
 */
size_t search_by_customer(const ComplaintList *list,
                          const char *customer_id,
                          const Complaint **out_results,
                          size_t max_results);

/**
 * @brief Searches complaints by Status.
 */
size_t search_by_status(const ComplaintList *list,
                        ComplaintStatus status,
                        const Complaint **out_results,
                        size_t max_results);

/**
 * @brief Searches complaints by Priority.
 */
size_t search_by_priority(const ComplaintList *list,
                          ComplaintPriority priority,
                          const Complaint **out_results,
                          size_t max_results);

/**
 * @brief Searches complaints by assigned Agent ID.
 */
size_t search_by_agent(const ComplaintList *list,
                       const char *agent_id,
                       const Complaint **out_results,
                       size_t max_results);

/**
 * @brief Searches complaints by substring keyword match in description.
 */
size_t search_by_keyword(const ComplaintList *list,
                         const char *keyword,
                         const Complaint **out_results,
                         size_t max_results);

/**
 * @brief Formats and displays an array of search result complaint pointers.
 */
void search_display_results(const Complaint **results, size_t count);

#endif /* SEARCH_H */
