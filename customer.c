#include "customer.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CUSTOMER_CAPACITY 16

static Customer *g_customers = NULL;
static size_t g_customer_count = 0;
static size_t g_customer_capacity = 0;
static uint32_t g_next_customer_id = 1001;

void customer_store_init(void) {
    if (g_customers == NULL) {
        g_customers = (Customer *)malloc(INITIAL_CUSTOMER_CAPACITY * sizeof(Customer));
        if (g_customers != NULL) {
            g_customer_capacity = INITIAL_CUSTOMER_CAPACITY;
            g_customer_count = 0;
            g_next_customer_id = 1001;
            memset(g_customers, 0, g_customer_capacity * sizeof(Customer));
        }
    }
}

void customer_store_cleanup(void) {
    if (g_customers != NULL) {
        free(g_customers);
        g_customers = NULL;
    }
    g_customer_count = 0;
    g_customer_capacity = 0;
    g_next_customer_id = 1001;
}

static SystemResult ensure_capacity(void) {
    if (g_customers == NULL) {
        customer_store_init();
        if (g_customers == NULL) {
            return SYS_ERR_NO_MEMORY;
        }
    }

    if (g_customer_count >= g_customer_capacity) {
        size_t new_cap = g_customer_capacity * 2;
        Customer *new_block = (Customer *)realloc(g_customers, new_cap * sizeof(Customer));
        if (new_block == NULL) {
            return SYS_ERR_NO_MEMORY;
        }
        g_customers = new_block;
        /* Zero out newly allocated slots */
        memset(g_customers + g_customer_capacity, 0, (new_cap - g_customer_capacity) * sizeof(Customer));
        g_customer_capacity = new_cap;
    }
    return SYS_OK;
}

SystemResult customer_add(const char *name,
                         const char *email,
                         const char *phone,
                         const char *address,
                         char *out_id) {
    if (name == NULL || phone == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    if (strlen(name) == 0 || strlen(phone) == 0) {
        return SYS_ERR_INVALID_INPUT;
    }

    /* Check duplicates by phone */
    if (customer_find_by_phone(phone) != NULL) {
        return SYS_ERR_DUPLICATE;
    }

    /* Check duplicates by email if provided */
    if (email != NULL && strlen(email) > 0 && strcmp(email, "N/A") != 0) {
        if (customer_find_by_email(email) != NULL) {
            return SYS_ERR_DUPLICATE;
        }
    }

    SystemResult cap_res = ensure_capacity();
    if (cap_res != SYS_OK) {
        return cap_res;
    }

    Customer *slot = &g_customers[g_customer_count];
    memset(slot, 0, sizeof(Customer));

    snprintf(slot->customer_id, sizeof(slot->customer_id), "CST-%04u", g_next_customer_id++);
    utils_safe_strcpy(slot->full_name, name, sizeof(slot->full_name));
    utils_safe_strcpy(slot->email, (email != NULL) ? email : "N/A", sizeof(slot->email));
    utils_safe_strcpy(slot->phone, phone, sizeof(slot->phone));
    utils_safe_strcpy(slot->address, (address != NULL) ? address : "N/A", sizeof(slot->address));
    slot->registered_at = utils_get_current_time();

    if (out_id != NULL) {
        utils_safe_strcpy(out_id, slot->customer_id, MAX_ID_LEN);
    }

    g_customer_count++;
    return SYS_OK;
}

const Customer* customer_find_by_id(const char *customer_id) {
    if (customer_id == NULL || g_customers == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < g_customer_count; i++) {
        if (strcmp(g_customers[i].customer_id, customer_id) == 0) {
            return &g_customers[i];
        }
    }
    return NULL;
}

const Customer* customer_find_by_phone(const char *phone) {
    if (phone == NULL || g_customers == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < g_customer_count; i++) {
        if (strcmp(g_customers[i].phone, phone) == 0) {
            return &g_customers[i];
        }
    }
    return NULL;
}

const Customer* customer_find_by_email(const char *email) {
    if (email == NULL || g_customers == NULL || strlen(email) == 0) {
        return NULL;
    }
    for (size_t i = 0; i < g_customer_count; i++) {
        if (strcmp(g_customers[i].email, email) == 0) {
            return &g_customers[i];
        }
    }
    return NULL;
}

size_t customer_get_all(const Customer **out_array) {
    if (out_array != NULL) {
        *out_array = g_customers;
    }
    return g_customer_count;
}

void customer_print_details(const Customer *c) {
    if (c == NULL) {
        printf("  [!] Customer record is NULL.\n");
        return;
    }
    char time_str[32];
    utils_format_datetime(c->registered_at, time_str, sizeof(time_str));

    printf("  +-----------------------------------------------------------+\n");
    printf("  | Customer ID   : %-41s |\n", c->customer_id);
    printf("  | Full Name     : %-41s |\n", c->full_name);
    printf("  | Phone Number  : %-41s |\n", c->phone);
    printf("  | Email Address : %-41s |\n", c->email);
    printf("  | Address       : %-41s |\n", c->address);
    printf("  | Registered On : %-41s |\n", time_str);
    printf("  +-----------------------------------------------------------+\n");
}

void customer_display_all(void) {
    if (g_customer_count == 0) {
        printf("\n  [i] No customers registered yet.\n");
        return;
    }

    printf("\n  ========================================================================================\n");
    printf("  %-10s | %-22s | %-14s | %-24s | %-12s\n", "ID", "Name", "Phone", "Email", "Registered");
    printf("  ----------------------------------------------------------------------------------------\n");

    for (size_t i = 0; i < g_customer_count; i++) {
        const Customer *c = &g_customers[i];
        char time_str[32];
        utils_format_datetime(c->registered_at, time_str, sizeof(time_str));
        time_str[10] = '\0'; /* Display date YYYY-MM-DD */

        printf("  %-10s | %-22.22s | %-14.14s | %-24.24s | %-12s\n",
               c->customer_id, c->full_name, c->phone, c->email, time_str);
    }
    printf("  ========================================================================================\n");
    printf("  Total Customers: %zu\n\n", g_customer_count);
}

size_t customer_get_count(void) {
    return g_customer_count;
}

const Customer* customer_get_at(size_t index) {
    if (index >= g_customer_count) {
        return NULL;
    }
    return &g_customers[index];
}

void customer_set_next_id_counter(uint32_t next_id) {
    g_next_customer_id = next_id;
}

uint32_t customer_get_next_id_counter(void) {
    return g_next_customer_id;
}

SystemResult customer_load_record(const Customer *record) {
    if (record == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    SystemResult res = ensure_capacity();
    if (res != SYS_OK) {
        return res;
    }
    g_customers[g_customer_count] = *record;
    g_customer_count++;

    /* Keep sequence counter aligned with loaded data */
    unsigned int num = 0;
    if (sscanf(record->customer_id, "CST-%04u", &num) == 1) {
        if (num >= g_next_customer_id) {
            g_next_customer_id = num + 1;
        }
    }
    return SYS_OK;
}
