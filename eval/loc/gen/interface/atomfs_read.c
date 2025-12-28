#include "interface.h"

/**
 * @brief Reads a specified number of bytes from a file at a given offset.
 *
 * @param path An array of strings representing the path to the file.
 * @param size The number of bytes to read from the file.
 * @param offset The offset within the file to start reading from.
 *
 * @return On success, returns a pointer to a `struct read_ret` which contains
 *         a buffer with the read data and the number of bytes read.
 * @return On failure, returns `NULL`. Failure can occur if the path does not
 *         exist, if the path points to a directory instead of a file, or if
 *         the file is not readable.
 */
struct read_ret* atomfs_read(char* path[], unsigned size, unsigned offset) {
    // Per the specification, acquire the lock for the root inode before starting the traversal.
    lock(root_inum);

    // Traverse the filesystem from the root to find the target inode.
    // The `locate` function handles releasing the lock on `root_inum` and
    // returns the `target` inode locked, satisfying its specification.
    struct inode* target = locate(root_inum, path);

    // If the path does not resolve to an inode, `locate` returns NULL.
    // Per its post-condition, all locks have already been released in this case.
    if (target == NULL) {
        return NULL;
    }

    // This block corresponds to "Case 1" in the specification.
    // We check if the located inode is a readable file. `target` is locked,
    // which satisfies the pre-condition of `check_file`.
    if (check_file(target) == 1) {
        // `check_file` returns 1 if `target` is not a readable file (e.g., a directory).
        // Per its specification, it releases the lock on `target` upon failure.
        // We can therefore return NULL directly, satisfying the post-condition
        // that no locks are held on exit.
        return NULL;
    }

    // This block corresponds to "Case 2" in the specification.
    // `check_file` returned 0, indicating success, and `target` is still locked.
    // We proceed to read the data from the inode.
    struct read_ret* result = inode_read(target, size, offset);

    // Release the lock on the target inode to satisfy the post-condition
    // of `atomfs_read` (no locks are owned upon exit).
    unlock(target);

    // Return the result of the read operation.
    return result;
}