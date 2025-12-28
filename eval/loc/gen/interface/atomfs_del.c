#include "interface.h"

/**
 * @brief Deletes a file or directory from the file system.
 *
 * This function traverses the file system starting from the root to locate the
 * parent directory specified by `path`. It then removes the directory entry
 * corresponding to `name` from this parent directory and deallocates the
 * associated inode and its resources.
 *
 * @param path A NULL-terminated string array representing the path to the
 *             parent directory of the entry to be deleted.
 * @param name The name of the file or directory to delete.
 *
 * @return 0 on successful deletion.
 * @return -1 on failure. Failure can occur if the path to the parent directory
 *         does not exist, if the deletion is not permitted (e.g., trying to
 *         delete a non-empty directory), or if an internal error occurs.
 *
 */
int atomfs_del(char* path[], char* name) {
    // Acquire the lock on the root inode to safely begin path traversal.
    // This is the entry point for any filesystem operation starting from the root.
    lock(root_inum);

    // Locate the parent directory inode. The `locate` function implements a
    // hand-over-hand locking protocol. It releases the lock on `root_inum`
    // and returns the locked inode for the parent directory if found.
    // If the path is not found, `locate` releases all locks and returns NULL.
    struct inode* parent = locate(root_inum, path);

    // Check if the parent directory was found.
    if (parent == NULL) {
        // Path traversal failed. `locate` has already released all locks.
        // We can safely return an error.
        return -1;
    }

    // Verify if the specified entry is allowed to be deleted.
    // Per its specification, `check_del` releases the lock on `parent`
    // if the check fails (returns non-zero), but keeps it if it succeeds.
    if (check_del(parent, name) != 0) {
        // Deletion is not permitted. The lock on `parent` has been released
        // by `check_del`. We can safely return an error.
        return -1;
    }

    // The deletion is permitted, and we still hold the lock on `parent`.
    // Proceed to remove the directory entry and retrieve the child inode.
    struct inode* child = inode_delete(parent, name);

    // Check if the inode deletion was successful.
    if (child == NULL) {
        // An internal error occurred during deletion. We must release the
        // lock on the parent directory before returning an error.
        unlock(parent);
        return -1;
    }

    // The entry has been removed from the parent directory.
    // Now, deallocate all resources associated with the deleted child inode.
    dispose_inode(child);

    // Release the lock on the parent directory, as the operation is complete.
    unlock(parent);

    // Return 0 to indicate successful deletion.
    return 0;
}