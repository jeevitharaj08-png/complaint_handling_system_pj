#ifndef STORAGE_H
#define STORAGE_H

#include "complaint.h"
#include <stdint.h>

/**
 * @brief Header block written at the beginning of binary data files for integrity checks.
 */
typedef struct {
    uint32_t magic;        /* Magic identifier (e.g. 0x434D5053) */
    uint32_t version;      /* Schema version */
    uint32_t record_count; /* Number of serialized fixed-size records */
    uint32_t next_id_seq;  /* Preserved ID counter sequence */
} StorageFileHeader;

/**
 * @brief Verifies and ensures the persistent runtime data/ directory exists.
 */
SystemResult storage_init_data_directory(void);

/**
 * @brief Safely replaces target_path with tmp_path across platforms (Windows & POSIX),
 *        preserving target_path if replacement fails, and cleaning up tmp_path on error.
 * @param tmp_path Path to the freshly committed temporary file.
 * @param target_path Destination target file path.
 * @return SYS_OK on success, or SYS_ERR_FILE_IO / SYS_ERR_NULL_PTR on error.
 */
SystemResult storage_replace_file(const char *tmp_path, const char *target_path);

/**
 * @brief Saves in-memory complaints doubly linked list to binary file.
 */
SystemResult storage_save_complaints(const ComplaintList *list);

/**
 * @brief Loads complaints from binary file into in-memory doubly linked list.
 */
SystemResult storage_load_complaints(ComplaintList *list);

/**
 * @brief Saves customers store to binary file.
 */
SystemResult storage_save_customers(void);

/**
 * @brief Loads customers from binary file into memory.
 */
SystemResult storage_load_customers(void);

/**
 * @brief Saves support agents store to binary file.
 */
SystemResult storage_save_agents(void);

/**
 * @brief Loads support agents from binary file into memory.
 */
SystemResult storage_load_agents(void);

/**
 * @brief Convenience function saving all components (complaints, customers, agents).
 */
SystemResult storage_save_all(const ComplaintList *list);

/**
 * @brief Convenience function loading all components.
 */
SystemResult storage_load_all(ComplaintList *list);

#endif /* STORAGE_H */
