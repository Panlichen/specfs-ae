#include "inode.h"

/**
 * @brief Changes the size of the file associated with an inode.
 *
 * This function adjusts the file size to the new specified `size`. It handles
 * both shrinking (truncation) and growing (extension) of the file.
 *
 * @param node A pointer to the inode whose file size is to be changed.
 *             This must be a valid pointer to an initialized inode structure.
 * @param size The new desired size for the file in bytes.
 *
 */
void inode_truncate(struct inode* node, unsigned size) {
    // Retrieve the current size of the file from the inode structure.
    unsigned current_size = node->size;

    // [SPECIFICATION] Post-Condition Case 1: If the new size is smaller than
    // the current size, truncate the file.
    if (size < current_size) {
        // Clear the data from the new end-of-file (`size`) to the old
        // end-of-file (`current_size`). The length of the block to clear is
        // `current_size - size`.
        file_clear(node, size, current_size - size);
        // Update the inode's metadata to reflect the new, smaller size.
        node->size = size;
    }
    // [SPECIFICATION] Post-Condition Case 2: If the new size is larger than
    // the current size, extend the file.
    else if (size > current_size) {
        // Allocate the additional space required. The allocation starts from the
        // old end-of-file (`current_size`) and has a length of `size - current_size`.
        file_allocate(node, current_size, size - current_size);
        // Clear the newly allocated space to ensure it doesn't contain stale data.
        // The region to clear starts at `current_size` and has a length of
        // `size - current_size`.
        file_clear(node, current_size, size - current_size);
        // Update the inode's metadata to reflect the new, larger size.
        node->size = size;
    }
    // If size == current_size, no action is performed, satisfying the logic
    // that no change is needed.
}