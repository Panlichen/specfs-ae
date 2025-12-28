/**
 * @brief Flushes a logged page update or deletion.
 * @param ref A pointer to the reference page in memory.
 * @param new A pointer to a new page with updated data, or NULL for deletion.
 *
 * If `new` is NULL, the reference page `ref` is freed.
 * If `new` is not NULL, its content is copied to `ref`, and `new` is freed.
 */
static void flush_log_page(unsigned char* ref, unsigned char* new) {
    if (new == NULL) {
        // Deletion: The page is no longer needed.
        free(ref);
    } else {
        // Update: Copy the new data into the original page.
        memcpy(ref, new, PAGE_SIZE);
        free(new); // Free the temporary new page.
    }
}

/**
 * @brief Flushes a logged directory entry update or deletion.
 * @param ref A pointer to the reference entry in memory.
 * @param new A pointer to a new entry with updated data, or NULL for deletion.
 *
 * If `new` is NULL, `ref` and its associated name are freed.
 * If `new` is not NULL, its data is transferred to `ref`, and `new` is freed.
 */
static void flush_log_entry(struct entry* ref, struct entry* new) {
    if (new == NULL) {
        // Deletion: Free the entry and its name.
        if (ref->name) {
            free(ref->name);
        }
        free(ref);
    } else {
        // Update: Free the old name and transfer ownership of the new data.
        if (ref->name) {
            free(ref->name);
        }
        ref->name = new->name; // Transfer name ownership
        ref->inum = new->inum;
        ref->next = new->next;
        
        // Free the container for the new entry, as its contents are now owned by ref.
        free(new);
    }
}

/**
 * @brief Flushes a logged inode update or deletion.
 * @param ref A pointer to the reference inode in memory.
 * @param new A pointer to a new inode with updated data, or NULL for deletion.
 *
 * Handles freeing or updating the inode and its associated directory/file tables.
 * Mutex-related fields are shallow-copied as specified.
 */
static void flush_log_inode(struct inode* ref, struct inode* new) {
    if (new == NULL) {
        // Deletion: Free the inode and its associated data table.
        if (ref->mode == DIR_MODE && ref->dir) {
            free(ref->dir);
        } else if (ref->mode == FILE_MODE && ref->file) {
            free(ref->file);
        }
        free(ref);
    } else {
        // Update: Free the old data table.
        if (ref->mode == DIR_MODE && ref->dir) {
            free(ref->dir);
        } else if (ref->mode == FILE_MODE && ref->file) {
            free(ref->file);
        }

        // Copy new inode data to the reference inode.
        // Shallow copy mutex fields.
        ref->mutex = new->mutex;
        ref->impl = new->impl;
        ref->hd = new->hd;
        
        // Copy remaining fields.
        ref->maj = new->maj;
        ref->min = new->min;
        ref->mode = new->mode;
        ref->size = new->size;
        
        // Transfer ownership of the new data tables.
        ref->dir = new->dir;
        ref->file = new->file;
        
        // Free the container for the new inode.
        free(new);
    }
}

/**
 * @brief Processes all pending records in the global log.
 *
 * Iterates through all write and delete logs, calling the appropriate
 * flush helper function for each record. Resets the log counters after processing.
 */
static void flush_log() {
    // Process all write log records
    for (int type = 0; type < LOG_TYPE_NUM; ++type) {
        for (int i = 0; i < log.wl[type].n; ++i) {
            void* ref_ptr = log.wl[type].record[i].ref_ptr;
            void* new_ptr = log.wl[type].record[i].new_ptr;
            
            switch (type) {
                case LOG_TYPE_DATA:
                    flush_log_page((unsigned char*)ref_ptr, (unsigned char*)new_ptr);
                    break;
                case LOG_TYPE_DIR:
                    flush_log_entry((struct entry*)ref_ptr, (struct entry*)new_ptr);
                    break;
                case LOG_TYPE_INODE:
                    flush_log_inode((struct inode*)ref_ptr, (struct inode*)new_ptr);
                    break;
            }
        }
        // Reset write log for this type
        log.wl[type].n = 0;
    }

    // Process all delete log records
    for (int type = 0; type < LOG_TYPE_NUM; ++type) {
        for (int i = 0; i < log.dl[type].n; ++i) {
            void* ref_ptr = log.dl[type].ref_ptr[i];
            
            switch (type) {
                case LOG_TYPE_DATA:
                    flush_log_page((unsigned char*)ref_ptr, NULL);
                    break;
                case LOG_TYPE_DIR:
                    flush_log_entry((struct entry*)ref_ptr, NULL);
                    break;
                case LOG_TYPE_INODE:
                    flush_log_inode((struct inode*)ref_ptr, NULL);
                    break;
            }
        }
        // Reset delete log for this type
        log.dl[type].n = 0;
    }
}

/**
 * @brief Commits the log by flushing all pending changes to disk.
 *
 * Checks if the log contains any pending operations. If it does,
 * it calls flush_log() to write them.
 */
static void log_commit() {
    int is_empty = 1;
    
    // Check if any of the logs have records
    for (int i = 0; i < LOG_TYPE_NUM; ++i) {
        if (log.wl[i].n > 0 || log.dl[i].n > 0) {
            is_empty = 0;
            break;
        }
    }
    
    // If the log is not empty, flush all changes
    if (!is_empty) {
        flush_log();
    }
}