#include "lowlevel_file.h"
#include "util.h"
#include <string.h> // For memcpy and memset
#include <stdio.h>  // For error reporting
#include <stdlib.h> // For malloc

/**
 * @brief Allocates external pages for file data that exceeds INLINE_DATA_SIZE.
 */
void file_allocate(struct inode *node, unsigned offset, unsigned len) {
    if (len == 0 || node->file == NULL) {
        return;
    }

    unsigned end_offset = offset + len;

    // If the entire requested range fits within the inline data, no external pages are needed.
    if (end_offset <= INLINE_DATA_SIZE) {
        return;
    }

    // Determine the starting file offset from which pages are required.
    // This is either the start of paged data or the user-provided offset, whichever is greater.
    unsigned alloc_start_offset = (offset > INLINE_DATA_SIZE) ? offset : INLINE_DATA_SIZE;

    // Calculate page indices relative to the start of the paged data area.
    unsigned start_page = (alloc_start_offset - INLINE_DATA_SIZE) / PG_SIZE;
    unsigned end_page = (end_offset - 1 - INLINE_DATA_SIZE) / PG_SIZE;

    for (unsigned page = start_page; page <= end_page; page++) {
        if (node->file->index[page] == NULL) {
            node->file->index[page] = malloc_page();
            if (node->file->index[page] == NULL) {
                perror("malloc_page failed in file_allocate");
                // In a real system, you would need more robust error handling,
                // possibly returning an error code.
                return;
            }
        }
    }
}

/**
 * @brief Reads data, transparently sourcing from inline_data or external pages.
 */
void file_read(struct inode *node, unsigned offset, unsigned len, char *data) {
    if (len == 0) return;

    unsigned current_offset = offset;
    unsigned remaining = len;
    char *data_ptr = data;

    // 1. Read the portion of data that falls within the inline buffer.
    if (current_offset < INLINE_DATA_SIZE) {
        unsigned bytes_to_read_inline = INLINE_DATA_SIZE - current_offset;
        if (bytes_to_read_inline > remaining) {
            bytes_to_read_inline = remaining;
        }

        memcpy(data_ptr, node->inline_data + current_offset, bytes_to_read_inline);

        data_ptr += bytes_to_read_inline;
        current_offset += bytes_to_read_inline;
        remaining -= bytes_to_read_inline;
    }

    // 2. Read any remaining data from external pages.
    if (remaining > 0 && node->file != NULL) {
        while (remaining > 0) {
            // Calculate offset and page index relative to the start of paged data.
            unsigned paged_offset = current_offset - INLINE_DATA_SIZE;
            unsigned page = paged_offset / PG_SIZE;
            unsigned offset_in_page = paged_offset % PG_SIZE;

            unsigned bytes_to_copy = PG_SIZE - offset_in_page;
            if (bytes_to_copy > remaining) {
                bytes_to_copy = remaining;
            }

            if (node->file->index[page] == NULL) {
                // If a page is not allocated (sparse file), read as zeros.
                memset(data_ptr, 0, bytes_to_copy);
            } else {
                memcpy(data_ptr, node->file->index[page] + offset_in_page, bytes_to_copy);
            }

            data_ptr += bytes_to_copy;
            current_offset += bytes_to_copy;
            remaining -= bytes_to_copy;
        }
    } else if (remaining > 0) {
        // The read extends beyond inline data, but there's no page table.
        // This part of the file is implicitly all zeros.
        memset(data_ptr, 0, remaining);
    }
}

/**
 * @brief Writes data, transparently targeting inline_data or external pages.
 */
void file_write(struct inode *node, unsigned offset, unsigned len, const char *data) {
    if (len == 0) return;
    
    // First, ensure all necessary external pages are allocated for the write operation.
    file_allocate(node, offset, len);

    unsigned current_offset = offset;
    unsigned remaining = len;
    const char *data_ptr = data;

    // 1. Write the portion of data that falls within the inline buffer.
    if (current_offset < INLINE_DATA_SIZE) {
        unsigned bytes_to_write_inline = INLINE_DATA_SIZE - current_offset;
        if (bytes_to_write_inline > remaining) {
            bytes_to_write_inline = remaining;
        }

        if (data_ptr == NULL) { // For clear_file
            memset(node->inline_data + current_offset, 0, bytes_to_write_inline);
        } else {
            memcpy(node->inline_data + current_offset, data_ptr, bytes_to_write_inline);
            data_ptr += bytes_to_write_inline;
        }

        current_offset += bytes_to_write_inline;
        remaining -= bytes_to_write_inline;
    }

    // 2. Write any remaining data to external pages.
    if (remaining > 0 && node->file != NULL) {
        while (remaining > 0) {
            unsigned paged_offset = current_offset - INLINE_DATA_SIZE;
            unsigned page = paged_offset / PG_SIZE;
            unsigned offset_in_page = paged_offset % PG_SIZE;

            unsigned bytes_to_copy = PG_SIZE - offset_in_page;
            if (bytes_to_copy > remaining) {
                bytes_to_copy = remaining;
            }
            
            // The page should exist due to the file_allocate call above.
            // A defensive check is still good practice.
            if (node->file->index[page] == NULL) {
                fprintf(stderr, "Error: Writing to a non-allocated page index %u.\n", page);
                // Skip this chunk to avoid a crash.
                if (data_ptr != NULL) data_ptr += bytes_to_copy;
                current_offset += bytes_to_copy;
                remaining -= bytes_to_copy;
                continue;
            }

            if (data_ptr == NULL) { // For clear_file
                memset(node->file->index[page] + offset_in_page, 0, bytes_to_copy);
            } else {
                memcpy(node->file->index[page] + offset_in_page, data_ptr, bytes_to_copy);
                data_ptr += bytes_to_copy;
            }
            
            current_offset += bytes_to_copy;
            remaining -= bytes_to_copy;
        }
    }
}

/**
 * @brief Clears (zeroes out) a region of the file.
 */
void clear_file(struct inode *node, unsigned start, unsigned len) {
    // This function requires no changes, as it relies on the updated file_write.
    file_write(node, start, len, NULL);
}