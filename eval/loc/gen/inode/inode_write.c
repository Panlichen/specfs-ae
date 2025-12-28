#include "inode.h"

/**
 * @brief Writes data from a buffer to an inode at a specified offset.
 *
 * @param node A pointer to a valid inode structure for the file.
 * @param buffer A pointer to the buffer containing the data to write.
 * @param len The number of bytes to write from the buffer.
 * @param offset The zero-based offset in the file where writing should begin.
 *
 * @return The number of bytes actually written. This value may be less than
 *         the requested `len` if the write was truncated to adhere to
 *         `MAX_FILE_SIZE`.
 */
unsigned inode_write(struct inode* node, const char* buffer, unsigned len, unsigned offset) {
    // Calculate the potential new size of the file if the write is completed.
    unsigned new_size = offset + len;

    // Per the specification, the file size cannot exceed MAX_FILE_SIZE.
    // If the calculated new size is too large, it must be truncated.
    if (new_size > MAX_FILE_SIZE) {
        // Cap the new file size at the maximum allowed limit.
        new_size = MAX_FILE_SIZE;

        // Adjust the write length (`len`) to fit within the file size limit.
        // If the offset is already at or beyond the max size, nothing can be written.
        // Otherwise, `len` becomes the remaining space from the offset to the max size.
        len = (offset <= MAX_FILE_SIZE) ? (MAX_FILE_SIZE - offset) : 0;
    }

    // If the new size extends beyond the current file size, the file must grow.
    // This block handles the allocation and initialization of the new space.
    if (new_size > node->size) {
        // The allocation must start from the current end of the file.
        unsigned allocate_from = node->size;
        // The amount of new space needed is the difference between the new and current sizes.
        unsigned allocate_len = new_size - node->size;

        // Allocate the additional data blocks required for the write.
        file_allocate(node, allocate_from, allocate_len);

        // Per the post-condition, clear the newly allocated space. This ensures
        // that file sections that are allocated but not yet written to
        // contain predictable data (e.g., zeros) instead of stale data.
        file_clear(node, allocate_from, allocate_len);

        // Update the inode's metadata to reflect the new, larger file size.
        node->size = new_size;
    }

    // Perform the actual write operation, copying `len` bytes from the buffer
    // into the file's data blocks starting at `offset`.
    file_write(node, offset, len, buffer);

    // Return the actual number of bytes written, which may have been truncated.
    return len;
}