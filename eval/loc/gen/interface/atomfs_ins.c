#include "interface.h"

/**
 * @brief Creates a new inode and inserts it into a specified directory.
 * @details This function implements the file system insertion operation. It first
 * traverses the given `path` to locate the target directory. If the directory
 * is found and the `name` is valid for insertion, it allocates a new inode
 * with the specified properties and inserts a new directory entry.
 *
 * @param path A NULL-terminated array of strings representing the path to the target directory.
 * @param name The name for the new entry to be created in the target directory.
 * @param mode The mode (e.g., file type, permissions) for the new inode.
 * @param maj The major device number for the new inode (if applicable).
 * @param min The minor device number for the new inode (if applicable).
 * @return 0 on successful insertion, -1 on failure.
 */
int atomfs_ins(char* path[], char* name, int mode, unsigned maj, unsigned min) {
    struct inode* target;
    
    // Per the refined specification, atomfs_ins starts with no locks owned.
    // We must lock the root inode to satisfy the pre-condition of `locate`.
    lock(root_inum);
    
    // Traverse the filesystem from the root to find the target directory inode.
    // According to its spec, `locate` will release the lock on `root_inum`
    // and acquire a lock on the returned `target` inode if successful.
    target = locate(root_inum, path);
    
    // Check for traversal failure (Case 2a).
    // If `locate` returns NULL, the path was not found. Its specification
    // guarantees that all locks have been released in this case.
    if (target == NULL) {
        return -1;
    }
    
    // The `target` inode is now locked. Check if the new `name` can be inserted.
    // This satisfies the pre-condition for `check_ins` (cur is locked).
    if (check_ins(target, name) != 0) {
        // Handle insertion check failure (Case 2b).
        // The spec for `check_ins` guarantees it releases the lock on `target`
        // if it returns a non-zero value, so no unlock is needed here.
        return -1;
    }
    
    // At this point, `check_ins` returned 0, so the `target` inode remains locked.
    // Allocate a new inode with the specified properties, as required for a successful insertion (Case 1).
    struct inode* new_inode = malloc_inode(mode, maj, min);

    // Handle potential memory allocation failure for the new inode.
    if (new_inode == NULL) {
        // We must manually release the lock on `target` before returning.
        unlock(target);
        return -1;
    }
    
    // Perform the actual insertion of the new entry `(name, new_inode)` into the target directory.
    int result = inode_insert(target, new_inode, name);
    
    // Release the lock on the target directory to satisfy the post-condition
    // that `atomfs_ins` leaves no locks held.
    unlock(target);
    
    // Map the result of `inode_insert` to the specified post-condition return values.
    // 0 indicates success (Case 1), -1 indicates failure.
    return result == 0 ? 0 : -1;
}