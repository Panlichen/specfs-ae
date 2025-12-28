#include "file.h"

/**
 * @brief Writes data to a file represented by an inode or zero-fills a specified range.
 *
 * This function handles writing data to a file's pages. It operates in two modes
 * based on the `data` parameter:
 *
 * 1.  **Data Write (`data` is not NULL)**: Copies `len` bytes from the `data` buffer
 *     to the file starting at `offset`. It is pre-conditioned that all necessary
 *     pages in the target range are already allocated.
 *
 * 2.  **Zero-Fill (`data` is NULL)**: Initializes `len` bytes to zero starting at
 *     `offset`. If a page within this range is not yet allocated, it will be
 *     allocated and then zero-filled.
 *
 * The function iterates through the target range, handling page boundaries correctly
 * until all `len` bytes have been processed.
 *
 * @param node A pointer to the inode structure representing the file. The inode's
 *             `file` member must be a valid indextb pointer.
 * @param offset The starting byte offset within the file to write to.
 * @param len The number of bytes to write.
 * @param data A pointer to the buffer containing the data to be written.
 *             If NULL, the specified range in the file is zero-filled.
 *
 */
void file_write(struct inode *node, unsigned offset, unsigned len, const char *data)
{
    // Initialize trackers for the current absolute write position and the
    // number of bytes remaining to be written.
    unsigned current_offset = offset;
    unsigned remaining = len;

    // Loop until all 'len' bytes have been processed.
    while (remaining > 0) {
        // Calculate the index of the page in the index table based on the current offset.
        unsigned page_index = current_offset / PG_SIZE;
        // Calculate the starting offset within that specific page.
        unsigned page_offset = current_offset % PG_SIZE;
        // Calculate the maximum number of bytes that can be written in the current page.
        unsigned bytes_in_page = PG_SIZE - page_offset;
        
        // If the remaining bytes to write are fewer than the available space in the page,
        // adjust the number of bytes to process in this iteration.
        if (bytes_in_page > remaining) {
            bytes_in_page = remaining;
        }

        // Retrieve the pointer to the data page from the index table.
        unsigned char *page = node->file->index[page_index];

        // Case 1: Zero-fill operation as specified when 'data' is NULL.
        if (data == NULL) {
            // As per the specification, if a page is unallocated, allocate it now.
            if (page == NULL) {
                page = (unsigned char *)malloc(PG_SIZE);
                // A production system should handle memory allocation failure gracefully.
                if (page == NULL) {
                    /* Handle allocation failure appropriately */
                    break;
                }
                // Store the newly allocated page back into the index table.
                node->file->index[page_index] = page;
            }
            // Write zeros to the calculated portion of the page.
            memset(page + page_offset, 0, bytes_in_page);
        } else {
            // Case 2: Standard data write operation.
            // Per the pre-condition, 'page' is assumed to be non-NULL here.
            // Copy a chunk of data into the correct position within the page.
            // The source address is advanced by the number of bytes already written.
            memcpy(page + page_offset, data + (current_offset - offset), bytes_in_page);
        }

        // Advance the current offset and decrement the remaining byte count
        // to prepare for the next iteration.
        current_offset += bytes_in_page;
        remaining -= bytes_in_page;
    }
}