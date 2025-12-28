#include "interface.h"

/**
 * @brief Truncates a file specified by a path to a given size.
 *
 * This function locates a file using the provided path and changes its size to the
 * specified offset.
 *
 * @param path A NULL-terminated array of strings representing the file path.
 * @param offset The new size for the file in bytes. The size must not exceed
 *               MAX_FILE_SIZE.
 * @return 0 on success.
 * @return -1 on failure if:
 *         a) The `offset` is greater than `MAX_FILE_SIZE`.
 *         b) The path traversal fails to find an inode.
 *         c) The target inode at the given path is not a regular file.
 */
int atomfs_truncate(char* path[], unsigned offset) {
    // Specification Case 2a: Validate the requested size against the file system limit.
    if (offset > MAX_FILE_SIZE) {
        return -1;
    }

    // Per `locate` pre-condition, the starting inode (`root_inum`) must be locked.
    lock(root_inum);

    // Traverse the path starting from the root to find the target inode.
    // `locate` releases the lock on `root_inum` and acquires a lock on `target`
    // if it is found.
    struct inode* target = locate(root_inum, path);

    // Specification Case 2b: If `locate` returns NULL, the path is invalid.
    // Per `locate` post-condition, no locks are held, so we can return safely.
    if (target == NULL) {
        return -1;
    }

    // Specification Case 2c: Verify that the target inode is a regular file.
    // `check_file` requires the inode to be locked.
    if (check_file(target) != 0) {
        // Per `check_file` post-condition, the lock on `target` is released on
        // failure, so we can return directly, satisfying the "no lock owned"
        // post-condition for `atomfs_truncate`.
        return -1;
    }

    // Perform the actual truncation. `inode_truncate` requires the `target`
    // inode to be locked and maintains the lock.
    inode_truncate(target, offset);

    // Release the lock on the target inode to satisfy the post-condition that
    // `atomfs_truncate` should not hold any locks upon successful exit.
    unlock(target);

    // Specification Case 1: Return 0 to indicate successful truncation.
    return 0;
}