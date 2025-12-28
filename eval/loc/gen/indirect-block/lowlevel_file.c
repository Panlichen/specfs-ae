#include "lowlevel_file.h"
#include <string.h> // for memcpy and memset
#include <stdio.h>  // for stderr
#include <stdlib.h> // for calloc

/**
 * @brief Mock function to allocate a zero-initialized page.
 * Uses calloc to ensure the returned memory is zeroed, which is crucial
 * for new indirect blocks so all initial pointers are NULL.
 * @return Pointer to the allocated page, or NULL on failure.
 */
void* malloc_page(void) {
    return calloc(1, PG_SIZE);
}


/**
 * @brief Retrieves the data page pointer for a given logical page number.
 * This function handles the logic for both direct and indirect lookups.
 * It does NOT allocate any pages.
 * @param tb Pointer to the file's index table.
 * @param page_num The logical page number to look up.
 * @return A pointer to the data page, or NULL if not allocated or out of bounds.
 */
static char* get_page(struct indextb *tb, unsigned page_num) {
    if (page_num >= MAX_PAGES) {
        return NULL; // Page number exceeds the maximum supported by single indirect block.
    }

    if (page_num < DIRECT_BLOCKS) {
        // --- Direct block access ---
        return tb->index[page_num];
    } else {
        // --- Indirect block access ---
        char *indirect_block_page = tb->index[DIRECT_BLOCKS];
        if (indirect_block_page == NULL) {
            return NULL; // Indirect block itself doesn't exist.
        }

        // Cast the indirect page to an array of char pointers
        char **indirect_block = (char **)indirect_block_page;
        // Calculate the index within the indirect block
        unsigned indirect_idx = page_num - DIRECT_BLOCKS;
        return indirect_block[indirect_idx];
    }
}

/**
 * @brief Retrieves or, if necessary, creates the data page for a given logical page number.
 * This function allocates the indirect block itself if it's the first time an indirect
 * page is requested.
 * @param tb Pointer to the file's index table.
 * @param page_num The logical page number to get or create.
 * @return A pointer to the data page, or NULL if the max file size limit is reached.
 */
static char* get_or_create_page(struct indextb *tb, unsigned page_num) {
    if (page_num >= MAX_PAGES) {
        return NULL; // Exceeds file size limit
    }

    if (page_num < DIRECT_BLOCKS) {
        // --- Direct block ---
        if (tb->index[page_num] == NULL) {
            tb->index[page_num] = malloc_page();
        }
        return tb->index[page_num];
    } else {
        // --- Indirect block ---
        // 1. Ensure the indirect block page itself is allocated
        if (tb->index[DIRECT_BLOCKS] == NULL) {
            tb->index[DIRECT_BLOCKS] = malloc_page();
            if (tb->index[DIRECT_BLOCKS] == NULL) return NULL; // Allocation failed
        }
        
        char **indirect_block = (char **)tb->index[DIRECT_BLOCKS];
        unsigned indirect_idx = page_num - DIRECT_BLOCKS;

        // 2. Ensure the specific data page is allocated
        if (indirect_block[indirect_idx] == NULL) {
            indirect_block[indirect_idx] = malloc_page();
        }
        return indirect_block[indirect_idx];
    }
}


void file_allocate(struct indextb *tb, unsigned offset, unsigned len) {
    if (len == 0) return;

    unsigned start_page = offset / PG_SIZE;
    unsigned end_page = (offset + len - 1) / PG_SIZE;

    for (unsigned page = start_page; page <= end_page; page++) {
        if (get_or_create_page(tb, page) == NULL) {
            // This occurs if we have hit MAX_PAGES
            fprintf(stderr, "Warning: File size limit reached. Could not allocate page %u.\n", page);
            break; // Stop allocating further
        }
    }
}

void file_read(struct indextb *tb, unsigned offset, unsigned len, char *data) {
    unsigned current_offset = offset;
    unsigned remaining = len;

    while (remaining > 0) {
        unsigned page = current_offset / PG_SIZE;
        unsigned offset_in_page = current_offset % PG_SIZE;
        unsigned bytes_to_read = PG_SIZE - offset_in_page;
        if (bytes_to_read > remaining) {
            bytes_to_read = remaining;
        }

        char *data_page = get_page(tb, page);

        if (data_page == NULL) {
            // If page doesn't exist, read as zeros
            memset(data, 0, bytes_to_read);
        } else {
            memcpy(data, data_page + offset_in_page, bytes_to_read);
        }

        data += bytes_to_read;
        current_offset += bytes_to_read;
        remaining -= bytes_to_read;
    }
}

void file_write(struct indextb *tb, unsigned offset, unsigned len, const char *data) {
    unsigned current_offset = offset;
    unsigned remaining = len;

    while (remaining > 0) {
        unsigned page = current_offset / PG_SIZE;
        unsigned offset_in_page = current_offset % PG_SIZE;
        unsigned bytes_to_write = PG_SIZE - offset_in_page;
        if (bytes_to_write > remaining) {
            bytes_to_write = remaining;
        }

        char *data_page;
        if (data == NULL) {
            // Special case for clear_file: get or create the page.
            data_page = get_or_create_page(tb, page);
        } else {
            // For a regular write, just get the page. Do not create.
            data_page = get_page(tb, page);
        }

        if (data_page == NULL) {
            // For regular writes, this means the page isn't pre-allocated.
            // For clear_file, this means we've hit the max file size.
            // In either case, we cannot write, so we advance to the next potential page.
        } else {
            if (data == NULL) {
                // `clear_file` operation: zero the memory range.
                memset(data_page + offset_in_page, 0, bytes_to_write);
            } else {
                // Regular write operation.
                memcpy(data_page + offset_in_page, data, bytes_to_write);
            }
        }

        if (data != NULL) {
            data += bytes_to_write;
        }
        current_offset += bytes_to_write;
        remaining -= bytes_to_write;
    }
}

void clear_file(struct inode *node, unsigned start, unsigned len) {
    // This function remains unchanged, as the allocation-on-clear logic
    // is now handled inside the updated file_write.
    file_write(node->file, start, len, NULL);
}