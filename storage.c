#include "storage.h"
#include "customer.h"
#include "agent.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define MAKE_DIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MAKE_DIR(d) mkdir(d, 0755)
#endif

SystemResult storage_init_data_directory(void) {
    MAKE_DIR(DATA_DIR);
    return SYS_OK;
}

SystemResult storage_replace_file(const char *tmp_path, const char *target_path) {
    if (tmp_path == NULL || target_path == NULL) {
        return SYS_ERR_NULL_PTR;
    }

#ifdef _WIN32
    /*
     * On Windows, MoveFileExA with MOVEFILE_REPLACE_EXISTING replaces target_path
     * without deleting it first. If target_path does not exist, it creates it.
     * If replacement fails (e.g. file locked or access denied), target_path
     * remains completely unmodified.
     */
    if (!MoveFileExA(tmp_path, target_path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        remove(tmp_path);
        return SYS_ERR_FILE_IO;
    }
    return SYS_OK;
#else
    /*
     * On POSIX systems, rename(src, dst) atomically replaces dst if dst exists,
     * without deleting dst beforehand. If rename fails, dst is unaffected.
     */
    if (rename(tmp_path, target_path) != 0) {
        remove(tmp_path);
        return SYS_ERR_FILE_IO;
    }
    return SYS_OK;
#endif
}

/* ========================================================================= */
/* Complaints Binary Storage                                                 */
/* ========================================================================= */

SystemResult storage_save_complaints(const ComplaintList *list) {
    if (list == NULL) {
        return SYS_ERR_NULL_PTR;
    }
    storage_init_data_directory();

    const char *tmp_path = "data/complaints.tmp";
    FILE *fp = fopen(tmp_path, "wb");
    if (fp == NULL) {
        return SYS_ERR_FILE_IO;
    }

    StorageFileHeader hdr;
    hdr.magic = MAGIC_COMPLAINTS;
    hdr.version = STORAGE_VERSION;
    hdr.record_count = (uint32_t)list->count;
    hdr.next_id_seq = list->next_id_counter;

    if (fwrite(&hdr, sizeof(StorageFileHeader), 1, fp) != 1) {
        fclose(fp);
        remove(tmp_path);
        return SYS_ERR_FILE_IO;
    }

    ComplaintNode *curr = list->head;
    while (curr != NULL) {
        ComplaintDiskRecord rec;
        complaint_to_disk_record(&curr->data, &rec);
        if (fwrite(&rec, sizeof(ComplaintDiskRecord), 1, fp) != 1) {
            fclose(fp);
            remove(tmp_path);
            return SYS_ERR_FILE_IO;
        }
        curr = curr->next;
    }

    if (fflush(fp) != 0) {
        fclose(fp);
        remove(tmp_path);
        return SYS_ERR_FILE_IO;
    }

    if (fclose(fp) != 0) {
        remove(tmp_path);
        return SYS_ERR_FILE_IO;
    }

    return storage_replace_file(tmp_path, FILE_COMPLAINTS_DAT);
}

SystemResult storage_load_complaints(ComplaintList *list) {
    if (list == NULL) {
        return SYS_ERR_NULL_PTR;
    }

    FILE *fp = fopen(FILE_COMPLAINTS_DAT, "rb");
    if (fp == NULL) {
        /* File does not exist yet; normal on first launch */
        return SYS_OK;
    }

    StorageFileHeader hdr;
    if (fread(&hdr, sizeof(StorageFileHeader), 1, fp) != 1) {
        fclose(fp);
        return SYS_ERR_FILE_IO;
    }

    if (hdr.magic != MAGIC_COMPLAINTS || hdr.version != STORAGE_VERSION) {
        fclose(fp);
        return SYS_ERR_INVALID_INPUT;
    }

    /* Reset existing list to avoid duplicates */
    complaint_list_cleanup(list);
    complaint_list_init(list);

    list->next_id_counter = hdr.next_id_seq;

    for (uint32_t i = 0; i < hdr.record_count; i++) {
        ComplaintDiskRecord rec;
        if (fread(&rec, sizeof(ComplaintDiskRecord), 1, fp) != 1) {
            fclose(fp);
            return SYS_ERR_FILE_IO;
        }
        complaint_load_disk_record(list, &rec);
    }

    fclose(fp);
    return SYS_OK;
}

/* ========================================================================= */
/* Customers Binary Storage                                                  */
/* ========================================================================= */

SystemResult storage_save_customers(void) {
    storage_init_data_directory();
    const char *tmp_path = "data/customers.tmp";
    FILE *fp = fopen(tmp_path, "wb");
    if (fp == NULL) {
        return SYS_ERR_FILE_IO;
    }

    size_t count = customer_get_count();
    StorageFileHeader hdr;
    hdr.magic = MAGIC_CUSTOMERS;
    hdr.version = STORAGE_VERSION;
    hdr.record_count = (uint32_t)count;
    hdr.next_id_seq = customer_get_next_id_counter();

    if (fwrite(&hdr, sizeof(StorageFileHeader), 1, fp) != 1) {
        fclose(fp);
        remove(tmp_path);
        return SYS_ERR_FILE_IO;
    }

    for (size_t i = 0; i < count; i++) {
        const Customer *c = customer_get_at(i);
        if (c != NULL) {
            if (fwrite(c, sizeof(Customer), 1, fp) != 1) {
                fclose(fp);
                remove(tmp_path);
                return SYS_ERR_FILE_IO;
            }
        }
    }

    if (fflush(fp) != 0) {
        fclose(fp);
        remove(tmp_path);
        return SYS_ERR_FILE_IO;
    }

    if (fclose(fp) != 0) {
        remove(tmp_path);
        return SYS_ERR_FILE_IO;
    }

    return storage_replace_file(tmp_path, FILE_CUSTOMERS_DAT);
}

SystemResult storage_load_customers(void) {
    FILE *fp = fopen(FILE_CUSTOMERS_DAT, "rb");
    if (fp == NULL) {
        return SYS_OK;
    }

    StorageFileHeader hdr;
    if (fread(&hdr, sizeof(StorageFileHeader), 1, fp) != 1) {
        fclose(fp);
        return SYS_ERR_FILE_IO;
    }

    if (hdr.magic != MAGIC_CUSTOMERS || hdr.version != STORAGE_VERSION) {
        fclose(fp);
        return SYS_ERR_INVALID_INPUT;
    }

    /* Reset existing store to avoid duplicates */
    customer_store_cleanup();
    customer_store_init();

    customer_set_next_id_counter(hdr.next_id_seq);

    for (uint32_t i = 0; i < hdr.record_count; i++) {
        Customer c;
        if (fread(&c, sizeof(Customer), 1, fp) != 1) {
            fclose(fp);
            return SYS_ERR_FILE_IO;
        }
        customer_load_record(&c);
    }

    fclose(fp);
    return SYS_OK;
}

/* ========================================================================= */
/* Agents Binary Storage                                                     */
/* ========================================================================= */

SystemResult storage_save_agents(void) {
    storage_init_data_directory();
    const char *tmp_path = "data/agents.tmp";
    FILE *fp = fopen(tmp_path, "wb");
    if (fp == NULL) {
        return SYS_ERR_FILE_IO;
    }

    size_t count = agent_get_count();
    StorageFileHeader hdr;
    hdr.magic = MAGIC_AGENTS;
    hdr.version = STORAGE_VERSION;
    hdr.record_count = (uint32_t)count;
    hdr.next_id_seq = agent_get_next_id_counter();

    if (fwrite(&hdr, sizeof(StorageFileHeader), 1, fp) != 1) {
        fclose(fp);
        remove(tmp_path);
        return SYS_ERR_FILE_IO;
    }

    for (size_t i = 0; i < count; i++) {
        const SupportAgent *a = agent_get_at(i);
        if (a != NULL) {
            if (fwrite(a, sizeof(SupportAgent), 1, fp) != 1) {
                fclose(fp);
                remove(tmp_path);
                return SYS_ERR_FILE_IO;
            }
        }
    }

    if (fflush(fp) != 0) {
        fclose(fp);
        remove(tmp_path);
        return SYS_ERR_FILE_IO;
    }

    if (fclose(fp) != 0) {
        remove(tmp_path);
        return SYS_ERR_FILE_IO;
    }

    return storage_replace_file(tmp_path, FILE_AGENTS_DAT);
}

SystemResult storage_load_agents(void) {
    FILE *fp = fopen(FILE_AGENTS_DAT, "rb");
    if (fp == NULL) {
        /* If no stored agents file, seed default staff */
        agent_seed_defaults();
        return SYS_OK;
    }

    StorageFileHeader hdr;
    if (fread(&hdr, sizeof(StorageFileHeader), 1, fp) != 1) {
        fclose(fp);
        agent_seed_defaults();
        return SYS_ERR_FILE_IO;
    }

    if (hdr.magic != MAGIC_AGENTS || hdr.version != STORAGE_VERSION) {
        fclose(fp);
        agent_seed_defaults();
        return SYS_ERR_INVALID_INPUT;
    }

    /* Reset existing store to avoid duplicates */
    agent_store_cleanup();
    agent_store_init();

    agent_set_next_id_counter(hdr.next_id_seq);

    for (uint32_t i = 0; i < hdr.record_count; i++) {
        SupportAgent a;
        if (fread(&a, sizeof(SupportAgent), 1, fp) != 1) {
            fclose(fp);
            return SYS_ERR_FILE_IO;
        }
        /* Reset in-memory workload counter because it will be dynamically recalculated
           as active complaints are loaded */
        a.active_tickets_count = 0;
        agent_load_record(&a);
    }

    fclose(fp);

    if (agent_get_count() == 0) {
        agent_seed_defaults();
    }

    return SYS_OK;
}

/* ========================================================================= */
/* Bulk Operations                                                           */
/* ========================================================================= */

SystemResult storage_save_all(const ComplaintList *list) {
    SystemResult r1 = storage_save_customers();
    SystemResult r2 = storage_save_agents();
    SystemResult r3 = storage_save_complaints(list);

    if (r1 != SYS_OK) return r1;
    if (r2 != SYS_OK) return r2;
    return r3;
}

SystemResult storage_load_all(ComplaintList *list) {
    SystemResult r1 = storage_load_customers();
    SystemResult r2 = storage_load_agents();
    SystemResult r3 = storage_load_complaints(list);

    if (r1 != SYS_OK) return r1;
    if (r2 != SYS_OK) return r2;
    return r3;
}
