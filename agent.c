#include "agent.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_AGENT_CAPACITY 16

static SupportAgent *g_agents = NULL;
static size_t g_agent_count = 0;
static size_t g_agent_capacity = 0;
static uint32_t g_next_agent_id = 1;

void agent_store_init(void) {
    if (g_agents == NULL) {
        g_agents = (SupportAgent *)malloc(INITIAL_AGENT_CAPACITY * sizeof(SupportAgent));
        if (g_agents != NULL) {
            g_agent_capacity = INITIAL_AGENT_CAPACITY;
            g_agent_count = 0;
            g_next_agent_id = 1;
            memset(g_agents, 0, g_agent_capacity * sizeof(SupportAgent));
        }
    }
}

void agent_store_cleanup(void) {
    if (g_agents != NULL) {
        free(g_agents);
        g_agents = NULL;
    }
    g_agent_count = 0;
    g_agent_capacity = 0;
    g_next_agent_id = 1;
}

static SystemResult ensure_capacity(void) {
    if (g_agents == NULL) {
        agent_store_init();
        if (g_agents == NULL) {
            return SYS_ERR_NO_MEMORY;
        }
    }

    if (g_agent_count >= g_agent_capacity) {
        size_t new_cap = g_agent_capacity * 2;
        SupportAgent *new_block = (SupportAgent *)realloc(g_agents, new_cap * sizeof(SupportAgent));
        if (new_block == NULL) {
            return SYS_ERR_NO_MEMORY;
        }
        g_agents = new_block;
        memset(g_agents + g_agent_capacity, 0, (new_cap - g_agent_capacity) * sizeof(SupportAgent));
        g_agent_capacity = new_cap;
    }
    return SYS_OK;
}

SystemResult agent_register(const char *name, const char *department, char *out_id) {
    if (name == NULL || department == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    if (strlen(name) == 0) {
        return SYS_ERR_INVALID_INPUT;
    }

    SystemResult res = ensure_capacity();
    if (res != SYS_OK) {
        return res;
    }

    SupportAgent *slot = &g_agents[g_agent_count];
    memset(slot, 0, sizeof(SupportAgent));

    snprintf(slot->agent_id, sizeof(slot->agent_id), "AGT-%02u", g_next_agent_id++);
    utils_safe_strcpy(slot->full_name, name, sizeof(slot->full_name));
    utils_safe_strcpy(slot->department, department, sizeof(slot->department));
    slot->is_active = true;
    slot->active_tickets_count = 0;

    if (out_id != NULL) {
        utils_safe_strcpy(out_id, slot->agent_id, MAX_ID_LEN);
    }

    g_agent_count++;
    return SYS_OK;
}

SupportAgent* agent_find_by_id(const char *agent_id) {
    if (agent_id == NULL || g_agents == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < g_agent_count; i++) {
        if (strcmp(g_agents[i].agent_id, agent_id) == 0) {
            return &g_agents[i];
        }
    }
    return NULL;
}

SupportAgent* agent_find_least_loaded(const char *optional_dept) {
    if (g_agents == NULL || g_agent_count == 0) {
        return NULL;
    }

    int min_tickets = 999999;
    for (size_t i = 0; i < g_agent_count; i++) {
        if (g_agents[i].is_active && g_agents[i].active_tickets_count < min_tickets) {
            min_tickets = g_agents[i].active_tickets_count;
        }
    }

    if (min_tickets == 999999) {
        return NULL;
    }

    /* Among agents holding the minimum workload, prefer a department specialist */
    if (optional_dept != NULL && strlen(optional_dept) > 0) {
        for (size_t i = 0; i < g_agent_count; i++) {
            if (g_agents[i].is_active &&
                g_agents[i].active_tickets_count == min_tickets &&
                strcmp(g_agents[i].department, optional_dept) == 0) {
                return &g_agents[i];
            }
        }
    }

    /* Otherwise return first active agent with that minimum workload */
    for (size_t i = 0; i < g_agent_count; i++) {
        if (g_agents[i].is_active && g_agents[i].active_tickets_count == min_tickets) {
            return &g_agents[i];
        }
    }

    return NULL;
}

void agent_increment_workload(const char *agent_id) {
    SupportAgent *agt = agent_find_by_id(agent_id);
    if (agt != NULL) {
        agt->active_tickets_count++;
    }
}

void agent_decrement_workload(const char *agent_id) {
    SupportAgent *agt = agent_find_by_id(agent_id);
    if (agt != NULL && agt->active_tickets_count > 0) {
        agt->active_tickets_count--;
    }
}

void agent_seed_defaults(void) {
    if (g_agent_count > 0) {
        return; /* Already populated */
    }
    agent_register("Dr. A. Sharma", "ACADEMIC", NULL);
    agent_register("Mr. R. Verma", "HOSTEL", NULL);
    agent_register("Er. S. Rao", "INFRASTRUCTURE", NULL);
    agent_register("Mrs. M. Patel", "FINANCE", NULL);
    agent_register("Prof. K. Iyer", "EXAMINATION", NULL);
    agent_register("Support Central", "OTHER", NULL);
}

void agent_display_all(void) {
    if (g_agent_count == 0) {
        printf("\n  [i] No agents registered.\n");
        return;
    }

    printf("\n  ========================================================================\n");
    printf("  %-10s | %-22s | %-16s | %-8s | %-12s\n", "ID", "Name", "Department", "Status", "Active Tickets");
    printf("  ------------------------------------------------------------------------\n");

    for (size_t i = 0; i < g_agent_count; i++) {
        const SupportAgent *agt = &g_agents[i];
        printf("  %-10s | %-22.22s | %-16.16s | %-8s | %-12d\n",
               agt->agent_id, agt->full_name, agt->department,
               agt->is_active ? "Active" : "Inactive",
               agt->active_tickets_count);
    }
    printf("  ========================================================================\n");
    printf("  Total Agents: %zu\n\n", g_agent_count);
}

size_t agent_get_count(void) {
    return g_agent_count;
}

const SupportAgent* agent_get_at(size_t index) {
    if (index >= g_agent_count) {
        return NULL;
    }
    return &g_agents[index];
}

void agent_set_next_id_counter(uint32_t next_id) {
    g_next_agent_id = next_id;
}

uint32_t agent_get_next_id_counter(void) {
    return g_next_agent_id;
}

SystemResult agent_load_record(const SupportAgent *record) {
    if (record == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    SystemResult res = ensure_capacity();
    if (res != SYS_OK) {
        return res;
    }
    g_agents[g_agent_count] = *record;
    g_agent_count++;

    unsigned int num = 0;
    if (sscanf(record->agent_id, "AGT-%02u", &num) == 1) {
        if (num >= g_next_agent_id) {
            g_next_agent_id = num + 1;
        }
    }
    return SYS_OK;
}
