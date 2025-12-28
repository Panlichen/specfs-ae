/**
 * @brief Adds a data page to the delete log.
 *
 * This function appends a pointer to a data page (or index table) to the
 * data-specific delete log. It is called internally when an inode's data
 * blocks are being deleted.
 *
 * @param page A void pointer to the page to be logged for deletion.
 * @pre The global log must be locked by the caller. Uniqueness check has been performed.
 */
static void log_delete_page(void* page) {
    // Get the delete log for data pages.
    struct delete_log* dl = &log.dl[LOG_TYPE_DATA];

    // Add the page pointer to the log and increment the count.
    // Assuming the log has enough space.
    if (dl->n < MAX_LOG_RECORD_NUM) {
        dl->ref_ptr[dl->n] = page;
        dl->n++;
    }
}

/**
 * @brief Adds a directory entry to the delete log.
 *
 * This function appends a pointer to an entry structure to the
 * directory-specific delete log.
 *
 * @param entry A pointer to the directory entry to be logged for deletion.
 * @pre The global log must be locked by the caller. Uniqueness check has been performed.
 */
static void log_delete_entry(struct entry* entry) {
    // Get the delete log for directory entries.
    struct delete_log* dl = &log.dl[LOG_TYPE_DIR];

    // Add the entry pointer to the log and increment the count.
    // Assuming the log has enough space.
    if (dl->n < MAX_LOG_RECORD_NUM) {
        dl->ref_ptr[dl->n] = entry;
        dl->n++;
    }
}

/**
 * @brief Adds an inode and its associated data pages to the delete log.
 *
 * This function logs an inode for deletion. If the inode represents a file,
 * it also iterates through its index table and logs each data page for deletion
 * by calling log_delete_page.
 *
 * @param inode A pointer to the inode to be logged for deletion.
 * @pre The global log must be locked by the caller. Uniqueness check has been performed.
 */
static void log_delete_inode(struct inode* inode) {
    // Get the delete log for inodes.
    struct delete_log* dl = &log.dl[LOG_TYPE_INODE];

    // Add the inode pointer to the log and increment the count.
    // Assuming the log has enough space.
    if (dl->n < MAX_LOG_RECORD_NUM) {
        dl->ref_ptr[dl->n] = inode;
        dl->n++;
    }

    // If the inode is a file, also log its data pages for deletion.
    if (inode->mode == FILE_MODE && inode->file != NULL) {
        for (int i = 0; i < INDEXTB_NUM; i++) {
            if (inode->file->index[i] != NULL) {
                // This call assumes the page isn't already in the log.
                // For a more robust system, a uniqueness check might be
                // needed here as well, but we follow the prompt's logic.
                log_delete_page(inode->file->index[i]);
            }
        }
    }
}

/**
 * @brief Logs a metadata object for deletion.
 *
 * This function serves as the main entry point for logging a deletion operation.
 * It first acquires a lock on the global log, then checks if the provided pointer
 * is already present in the log to avoid duplicates. If not, it dispatches to the
 * appropriate type-specific logging function.
 *
 * @param ptr A void pointer to the metadata object to be deleted.
 * @param type The type of the metadata object (e.g., LOG_TYPE_INODE).
 */
void log_delete(void* ptr, int type) {
    if (ptr == NULL) {
        return;
    }

    // Acquire the lock to ensure thread-safe access to the log.
    log_lock(&log);

    // Select the appropriate delete log based on the type.
    struct delete_log* dl = &log.dl[type];

    // Scan the log to check if this pointer has already been logged.
    for (int i = 0; i < dl->n; i++) {
        if (dl->ref_ptr[i] == ptr) {
            // Pointer already exists, do nothing.
            log_unlock(&log);
            return;
        }
    }

    // If the pointer is not in the log, add it by calling the specific helper.
    switch (type) {
        case LOG_TYPE_DIR:
            log_delete_entry((struct entry*)ptr);
            break;
        case LOG_TYPE_INODE:
            log_delete_inode((struct inode*)ptr);
            break;
        case LOG_TYPE_DATA:
            log_delete_page(ptr);
            break;
        default:
            // Unrecognized type, do nothing.
            break;
    }

    // Release the lock after modifying the log.
    log_unlock(&log);
}