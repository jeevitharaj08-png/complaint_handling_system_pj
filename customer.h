#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "common.h"
#include <stddef.h>

/**
 * @brief Customer entity structure.
 */
typedef struct {
    char customer_id[MAX_ID_LEN];
    char full_name[MAX_NAME_LEN];
    char email[MAX_EMAIL_LEN];
    char phone[MAX_PHONE_LEN];
    char address[MAX_ADDR_LEN];
    DateTime registered_at;
} Customer;

/**
 * @brief Initializes customer in-memory store.
 */
void customer_store_init(void);

/**
 * @brief Releases all allocated memory used by the customer store.
 */
void customer_store_cleanup(void);

/**
 * @brief Registers a new customer into the system.
 * @param name Full name.
 * @param email Email address.
 * @param phone Contact phone number.
 * @param address Physical address.
 * @param out_id Output buffer for generated customer ID (must be at least MAX_ID_LEN).
 * @return SYS_OK on success, or error code.
 */
SystemResult customer_add(const char *name,
                         const char *email,
                         const char *phone,
                         const char *address,
                         char *out_id);

/**
 * @brief Finds customer by unique Customer ID.
 */
const Customer* customer_find_by_id(const char *customer_id);

/**
 * @brief Finds customer by phone number.
 */
const Customer* customer_find_by_phone(const char *phone);

/**
 * @brief Finds customer by email.
 */
const Customer* customer_find_by_email(const char *email);

/**
 * @brief Gets pointer to all customers and count.
 */
size_t customer_get_all(const Customer **out_array);

/**
 * @brief Formats and prints a single customer's card to stdout.
 */
void customer_print_details(const Customer *c);

/**
 * @brief Displays a tabular list of all registered customers.
 */
void customer_display_all(void);

/**
 * @brief Direct access to internal count and buffer for serialization.
 */
size_t customer_get_count(void);
const Customer* customer_get_at(size_t index);
void customer_set_next_id_counter(uint32_t next_id);
uint32_t customer_get_next_id_counter(void);
SystemResult customer_load_record(const Customer *record);

#endif /* CUSTOMER_H */
