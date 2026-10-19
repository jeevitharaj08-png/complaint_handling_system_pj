#ifndef AGENT_H
#define AGENT_H

#include "common.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Support Agent entity structure.
 */
typedef struct {
    char agent_id[MAX_ID_LEN];
    char full_name[MAX_NAME_LEN];
    char department[MAX_CATEGORY_LEN];
    bool is_active;
    int active_tickets_count;
} SupportAgent;

/**
 * @brief Initializes support agent store.
 */
void agent_store_init(void);

/**
 * @brief Releases agent store resources.
 */
void agent_store_cleanup(void);

/**
 * @brief Populates standard college support agents if store is empty.
 */
void agent_seed_defaults(void);

/**
 * @brief Registers a new support agent.
 * @param name Agent full name.
 * @param department Primary department / category specialty.
 * @param out_id Output buffer for generated agent ID (`AGT-XX`).
 * @return SYS_OK on success.
 */
SystemResult agent_register(const char *name, const char *department, char *out_id);

/**
 * @brief Finds an agent by Agent ID.
 * @return Pointer to mutable SupportAgent, or NULL if not found.
 */
SupportAgent* agent_find_by_id(const char *agent_id);

/**
 * @brief Selects the active agent with the fewest active complaints.
 * @param optional_dept If provided, prioritizes agents matching this department.
 *                      Falls back to any active agent if department has no active agents.
 * @return Pointer to least-loaded SupportAgent, or NULL if no agents exist.
 */
SupportAgent* agent_find_least_loaded(const char *optional_dept);

/**
 * @brief Increments active ticket count for the agent.
 */
void agent_increment_workload(const char *agent_id);

/**
 * @brief Decrements active ticket count for the agent (floor at 0).
 */
void agent_decrement_workload(const char *agent_id);

/**
 * @brief Displays a tabular list of all agents and their live workloads.
 */
void agent_display_all(void);

/**
 * @brief Direct serialization helpers.
 */
size_t agent_get_count(void);
const SupportAgent* agent_get_at(size_t index);
void agent_set_next_id_counter(uint32_t next_id);
uint32_t agent_get_next_id_counter(void);
SystemResult agent_load_record(const SupportAgent *record);

#endif /* AGENT_H */
