/**
 * @brief Creates a deep copy of a directory entry structure.
 * @param entry The original directory entry to copy.
 * @return A pointer to the newly allocated copy, or NULL on failure.
 */
static struct entry* log_copy_entry(struct entry* entry) {
    if (!entry) {
        return NULL;
    }

    // Allocate a new entry structure.
    struct entry* new_entry = malloc_entry();
    if (!new_entry) {
        return NULL; // Allocation failed.
    }

    // Deep copy the name string.
    if (entry->name) {
        new_entry->name = malloc_string(entry->name);
        if (!new_entry->name) {
            free(new_entry); // Clean up partially allocated structure.
            return NULL;
        }
    } else {
        new_entry->name = NULL;
    }
    
    // Shallow copy the remaining fields as required.
    new_entry->inum = entry->inum;
    new_entry->next = entry->next;

    return new_entry;
}

/**
 * @brief Creates a copy of an inode structure.
 * @param inode The original inode to copy.
 * @return A pointer to the newly allocated copy, or NULL on failure.
 */
static struct inode* log_copy_inode(struct inode* inode) {
    if (!inode) {
        return NULL;
    }

    // Allocate memory for the new inode structure.
    struct inode* new_inode = (struct inode*)malloc(sizeof(struct inode));
    if (!new_inode) {
        return NULL; // Allocation failed.
    }

    // Shallow copy mutex-related and primitive fields.
    new_inode->mutex = inode->mutex;
    new_inode->impl = inode->impl;
    new_inode->hd = inode->hd;
    new_inode->maj = inode->maj;
    new_inode->min = inode->min;
    new_inode->mode = inode->mode;
    new_inode->size = inode->size;

    new_inode->dir = NULL;
    new_inode->file = NULL;

    // Copy the directory or file table based on the inode's mode.
    if (inode->mode == DIR_MODE && inode->dir) {
        new_inode->dir = (struct dirtb*)malloc(sizeof(struct dirtb));
        if (!new_inode->dir) {
            free(new_inode);
            return NULL;
        }
        // Perform a shallow copy of the entry pointer table.
        memcpy(new_inode->dir->tb, inode->dir->tb, sizeof(inode->dir->tb));
    } else if (inode->mode == FILE_MODE && inode->file) {
        new_inode->file = (struct indextb*)malloc(sizeof(struct indextb));
        if (!new_inode->file) {
            free(new_inode);
            return NULL;
        }
        // Perform a shallow copy of the page pointer table.
        memcpy(new_inode->file->index, inode->file->index, sizeof(inode->file->index));
    }

    return new_inode;
}

/**
 * @brief Creates a complete copy of a 4KB data page.
 * @param page The original page to copy.
 * @return A pointer to the newly allocated copy, or NULL on failure.
 */
static unsigned char* log_copy_page(unsigned char* page) {
    if (!page) {
        return NULL;
    }

    // Allocate a new 4KB page.
    unsigned char* new_page = malloc_page();
    if (!new_page) {
        return NULL; // Allocation failed.
    }

    // Copy the entire contents of the original page.
    memcpy(new_page, page, PAGE_SIZE);

    return new_page;
}

/**
 * @brief Retrieves a pointer to a logged (copied) data structure.
 *
 * This function first checks if a copy of the object at `ptr` already exists
 * in the log. If it does, a pointer to that copy is returned. If not, it
 * creates a new copy using the appropriate helper function, records it in
 * the log, and then returns a pointer to the new copy.
 *
 * @param ptr A pointer to the original data structure.
 * @param type The type of the data structure (LOG_TYPE_DATA, LOG_TYPE_DIR, LOG_TYPE_INODE).
 * @return A pointer to the copied data structure in the log, or NULL on failure.
 */
void* log_get(void* ptr, int type) {
    if (!ptr) {
        return NULL;
    }

    // Acquire lock to safely access the shared log structure.
    log_lock(&log);

    // First, scan the log to check if this pointer has already been copied.
    struct write_log* wlog = &log.wl[type];
    for (int i = 0; i < wlog->n; i++) {
        if (wlog->record[i].ref_ptr == ptr) {
            // Found an existing log entry. Return the pointer to the copy.
            void* new_ptr = wlog->record[i].new_ptr;
            log_unlock(&log);
            return new_ptr;
        }
    }

    // If we reach here, the pointer is not in the log. We must create a copy.
    void* new_ptr = NULL;
    switch (type) {
        case LOG_TYPE_DATA:
            new_ptr = log_copy_page((unsigned char*)ptr);
            break;
        case LOG_TYPE_DIR: // Corresponds to 'entry'
            new_ptr = log_copy_entry((struct entry*)ptr);
            break;
        case LOG_TYPE_INODE:
            new_ptr = log_copy_inode((struct inode*)ptr);
            break;
    }

    if (!new_ptr) {
        // The copy operation failed (e.g., out of memory).
        log_unlock(&log);
        return NULL;
    }

    // Check if there is space to add the new record to the log.
    if (wlog->n >= MAX_LOG_RECORD_NUM) {
        // The log is full. This is a critical error.
        // For this implementation, we abort the operation.
        // Note: new_ptr is leaked here, as we lack a generic free mechanism.
        // A more robust system would require type-aware cleanup.
        log_unlock(&log);
        return NULL; 
    }

    // Store the original pointer and the new copy's pointer in the log.
    wlog->record[wlog->n].ref_ptr = ptr;
    wlog->record[wlog->n].new_ptr = new_ptr;
    wlog->n++;

    // Release the lock before returning.
    log_unlock(&log);

    return new_ptr;
}