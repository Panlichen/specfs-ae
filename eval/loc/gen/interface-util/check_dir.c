#include "interface-util.h"

/**
 * @brief Validates if a given inode represents a directory.
 *
 *
 * @param inum A pointer to the `struct inode` to be validated. It can be `NULL`.
 *
 * @return 0 on success, indicating `inum` is a valid directory. In this case,
 *         the lock for `inum` remains held by the current thread.
 * @return 1 on failure, indicating `inum` is `NULL` or not a directory. If
 *         `inum` was not `NULL` upon entry, its lock is released before returning.
 */
int check_dir(struct inode *inum)
{
    // Specification Case 2: Handle failure where the provided inode pointer is NULL.
    // According to the post-condition, return 1. No lock needs to be released.
    if (inum == NULL) {
        return 1;
    }

    // Specification Case 1: Check if the inode's mode is DIR_MODE for success.
    if (inum->mode == DIR_MODE) {
        // The inode is a valid directory.
        // Per the post-condition, the lock remains held. Return 0 for success.
        return 0;
    } else {
        // Specification Case 2: Handle failure where the inode is not a directory.
        // Per the refined post-condition, the lock must be released.
        unlock(inum);
        // Return 1 to indicate failure.
        return 1;
    }
}