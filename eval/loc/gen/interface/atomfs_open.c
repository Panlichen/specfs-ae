#include "interface.h"

/**
 * @brief Opens a file specified by a path and checks access permissions.
 *
 * This function traverses the file system starting from the root directory to locate the
 * inode corresponding to the given path. It then checks if the requested open `mode` is
 * permissible for that inode.
 *
 * @param path A NULL-terminated array of strings representing the path components.
 * @param mode The desired access mode for opening the file.
 *
 * @return On success, returns a pointer to the located inode.
 * @return On failure, returns NULL. Failure can occur if the path does not exist or
 *         if the requested access mode is not permitted.
 *
 */
struct inode *atomfs_open(char *path[], unsigned mode)
{
    struct inode *target;

    // The locate() function requires its first argument (the starting directory) to be locked.
    // We start traversal from the root inode.
    lock(root_inum);

    // Traverse the filesystem from the root using the given path.
    // According to its specification, locate() will release the lock on `root_inum`.
    // If an inode is found, it will be locked and returned. If not, NULL is returned
    // and no locks are held.
    target = locate(root_inum, path);

    // [SPEC] Case 2a: The path traversal fails.
    if (target == NULL) {
        // `locate` guarantees no locks are held on failure, so we can return directly.
        return NULL;
    }

    // At this point, the `target` inode has been found and is locked.

    // Check if the requested open mode is permitted for the target inode.
    // The check_open() function requires the inode to be locked.
    // Crucially, its specification states it will unlock the inode before returning,
    // regardless of the check's outcome.
    if (check_open(target, mode) != 0) {
        // [SPEC] Case 2b: The open check fails.
        // `check_open` has already released the lock on `target`, so we can return.
        return NULL;
    }

    // [SPEC] Case 1: Successful open.
    // The path was found and the permissions check passed.
    // The lock on `target` has been released by `check_open`, satisfying the
    // post-condition that `atomfs_open` holds no locks on exit.
    return target;
}