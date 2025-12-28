#include "interface.h"

/**
 * @brief Get the attributes of an inode specified by a path.
 *
 * This function implements the `getattr` operation. It traverses the file system
 * starting from the `root_inum` to find the inode corresponding to the given
 * `path`. If found, it allocates a `getattr_ret` structure and populates it
 * with the inode's attributes.
 *
 * @param path A NULL-terminated array of strings representing the path to the
 *             target inode. This corresponds to the `path` pre-condition.
 * @return On successful lookup, returns a pointer to a new `getattr_ret`
 *         structure containing the inode's attributes (`mode`, `size`, `maj`, `min`).
 *         This fulfills "Case 1 (Successful lookup)" of the post-condition.
 * @return On lookup failure (if the path does not exist), returns `NULL`. This
 *         fulfills "Case 2 (Lookup failure)" of the post-condition.
 *
 */
struct getattr_ret* atomfs_getattr(char* path[]) {
    // Per the specification, `atomfs_getattr` starts with no locks held.
    // We must lock `root_inum` to satisfy the pre-condition of `locate`, which
    // expects its `cur` argument to be locked.
    lock(root_inum);

    // Traverse the path starting from the root directory to find the target inode.
    // As per `locate`'s specification, `locate` will release the lock on
    // `root_inum`. If an inode is found, `locate` returns a pointer to it with its
    // lock acquired. If not found, it returns NULL and releases all locks.
    struct inode* target = locate(root_inum, path);
    
    // Handle "Case 2 (Lookup failure)" from the specification.
    // If `locate` returned NULL, the path was not found. In this case, `locate`
    // has already released all locks, so we can return NULL directly.
    if (target == NULL) {
        return NULL;
    }
    
    // This block handles "Case 1 (Successful lookup)". The target inode was found
    // and its lock is currently held.

    // Allocate a `getattr_ret` structure and populate it with the attributes
    // from the found inode (`mode`, `size`, `maj`, `min`).
    struct getattr_ret* ret = malloc_getattr_ret(
        target, target->mode, target->size, target->maj, target->min
    );

    // Fulfill the post-condition of `atomfs_getattr`: no locks should be held upon exit.
    // Since `locate` returned a locked inode on success, we must now release it.
    unlock(target);
    
    // Return the populated attribute structure to the caller.
    return ret;
}