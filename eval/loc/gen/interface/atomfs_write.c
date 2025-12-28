#include "interface.h"

/**
 * @brief Writes data to a file at a specified offset.
 * 
 * This function locates the inode corresponding to the given path, verifies that it
 * is a regular file, and then writes the provided data into it at the specified offset.
 *
 * @param path A null-terminated array of strings representing the path to the file.
 * @param buf A pointer to the buffer containing the data to be written.
 * @param size The number of bytes to write from the buffer.
 * @param offset The offset within the file at which to start writing.
 *
 *
 * @return
 * - On success, returns the number of bytes written (the result of `inode_write`).
 * - On failure, returns -1 if:
 *   - The path does not resolve to a valid inode.
 *   - The located inode is not a writable file (e.g., it is a directory).
 */
int atomfs_write(char* path[], const char* buf, unsigned size, unsigned offset)
{
    // Per the [SPECIFICATION of locate], the 'cur' inode (root_inum here) must be locked.
    lock(root_inum);

    // Traverse the filesystem from the root to find the target inode.
    // According to its specification, `locate` will release the lock on `root_inum`
    // and acquire the lock on the returned inode `inum`. If no inode is found,
    // all locks are released, and NULL is returned.
    struct inode *inum = locate(root_inum, path);

    // If locate returns NULL, the path is invalid. No locks are held.
    if (inum == NULL) {
        return -1;
    }
    
    // Per the [SPECIFICATION of check_file], the inode `inum` must be locked.
    // This condition is met as `locate` locks the returned inode.
    // `check_file` verifies that `inum` is a regular file and not a directory.
    int res = check_file(inum);

    // [SPECIFICATION] Case 1: The located inode is not a writable file.
    // `check_file` returns 1 for failure. On failure, its specification guarantees
    // it releases the lock on `inum`. Thus, we can return directly.
    if (res == 1) {
        return -1;
    }

    // [SPECIFICATION] Case 2: The located inode is a writable file.
    // `check_file` returned 0, and `inum` is still locked.
    // Proceed to write the data to the file's inode.
    int ret = inode_write(inum, buf, size, offset);

    // To satisfy the post-condition that no locks are held upon exit,
    // we must release the lock on `inum` that was acquired by `locate`.
    unlock(inum);

    // Return the result of the write operation as specified.
    return ret;
}