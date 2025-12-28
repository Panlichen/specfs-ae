#include <string.h> // For memcpy, memset
#include <stdlib.h> // For NULL

#include "delalloc.h"


// Global instance of the allocation buffer.
// It is initialized to all zeros by default for global variables.
struct alloc_buffer alloc_buffer;

// Provided Utilities from util.h (do not reimplement).
// These are assumed to be implemented in another file.
extern unsigned char* malloc_page(void);
extern void buffer_lock(struct alloc_buffer* buf);
extern void buffer_unlock(struct alloc_buffer* buf);


/**
 * @brief Flushes all buffered writes to the underlying storage.
 *
 * This function iterates through all cached blocks, allocates physical pages
 * if they don't exist, and copies the data from the buffer to the physical pages.
 * After flushing, it resets the buffer.
 *
 * PRECONDITION: The caller must hold the lock on `alloc_buffer`.
 */
static void bflush() {
    for (unsigned int i = 0; i < alloc_buffer.size; ++i) {
        struct record *r = &alloc_buffer.rec[i];
        struct indextb *tb = r->tb;
        unsigned int page = r->page;

        // Allocate a page in the index table if it's a new page
        if (tb->index[page] == NULL) {
            tb->index[page] = malloc_page();
            if (tb->index[page] == NULL) {
                // Skip if page allocation fails
                continue; 
            }
        }

        // Copy data from the buffer to the corresponding page
        memcpy(tb->index[page], &alloc_buffer.buf[r->buf_offset * PAGE_SIZE], PAGE_SIZE);
    }
    // Reset the buffer size, effectively clearing it
    alloc_buffer.size = 0;
}


/**
 * @brief Reads data from a page, checking the write buffer first.
 */
unsigned char *bread(struct indextb *tb, unsigned int page) {
    buffer_lock(&alloc_buffer);

    unsigned char *result = NULL;

    // First, search the allocation buffer for the page
    for (unsigned int i = 0; i < alloc_buffer.size; ++i) {
        if (alloc_buffer.rec[i].tb == tb && alloc_buffer.rec[i].page == page) {
            result = &alloc_buffer.buf[alloc_buffer.rec[i].buf_offset * PAGE_SIZE];
            // Found in buffer, unlock and return
            buffer_unlock(&alloc_buffer);
            return result;
        }
    }

    // Not found in buffer, so return the pointer from the index table
    result = tb->index[page];

    buffer_unlock(&alloc_buffer);
    return result;
}

/**
 * @brief Writes data to a page via the delayed allocation buffer.
 */
void bwrite(struct indextb *tb, unsigned int page, unsigned int offset, unsigned char *buf, unsigned int size) {
    buffer_lock(&alloc_buffer);

    // Search if the page is already cached in the buffer
    for (unsigned int i = 0; i < alloc_buffer.size; ++i) {
        if (alloc_buffer.rec[i].tb == tb && alloc_buffer.rec[i].page == page) {
            unsigned char *dest = &alloc_buffer.buf[alloc_buffer.rec[i].buf_offset * PAGE_SIZE] + offset;
            if (buf) {
                memcpy(dest, buf, size);
            } else {
                memset(dest, 0, size);
            }
            buffer_unlock(&alloc_buffer);
            return;
        }
    }

    // Page is not in the buffer, a new buffer block must be allocated.

    // If the buffer is full, flush it to make space.
    if (alloc_buffer.size >= ALLOC_BUFFER_BLOCK_NUM) {
        bflush();
    }

    // Allocate a new block in the buffer
    unsigned int new_idx = alloc_buffer.size;
    unsigned char *dest_page = &alloc_buffer.buf[new_idx * PAGE_SIZE];

    // If the page already exists on "disk" (in the index table),
    // we must first read its old content into the buffer to preserve it.
    if (tb->index[page] != NULL) {
        memcpy(dest_page, tb->index[page], PAGE_SIZE);
    } else {
        // Otherwise, it's a new page, so we zero the buffer block to avoid stale data.
        memset(dest_page, 0, PAGE_SIZE);
    }
    
    // Perform the write on the buffered page
    unsigned char *dest_write = dest_page + offset;
    if (buf) {
        memcpy(dest_write, buf, size);
    } else {
        memset(dest_write, 0, size);
    }

    // Create a new record for this buffered block
    alloc_buffer.rec[new_idx].tb = tb;
    alloc_buffer.rec[new_idx].page = page;
    alloc_buffer.rec[new_idx].buf_offset = new_idx;

    alloc_buffer.size++;

    buffer_unlock(&alloc_buffer);
}

/**
 * @brief Releases (discards) all buffered writes for a given index table.
 */
void brels(struct indextb *tb) {
    buffer_lock(&alloc_buffer);

    unsigned int i = 0;
    while (i < alloc_buffer.size) {
        if (alloc_buffer.rec[i].tb == tb) {
            // Found a record to remove.
            // Overwrite it with the last record in the buffer to avoid a hole.
            alloc_buffer.size--;
            
            if (i < alloc_buffer.size) {
                // Move the last record's metadata
                alloc_buffer.rec[i] = alloc_buffer.rec[alloc_buffer.size];
                
                // The buf_offset of the moved record must be updated to its new index
                alloc_buffer.rec[i].buf_offset = i;

                // Move the last page's data into the slot of the removed page
                memcpy(&alloc_buffer.buf[i * PAGE_SIZE], &alloc_buffer.buf[alloc_buffer.size * PAGE_SIZE], PAGE_SIZE);
            }
            // Do not increment 'i' because the new element at 'i' must also be checked.
        } else {
            // This record is not for the target 'tb', so skip it.
            i++;
        }
    }

    buffer_unlock(&alloc_buffer);
}