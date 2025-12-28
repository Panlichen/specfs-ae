#include "interface.h"

/**
 * @brief Atomically renames or moves a file or directory.
 *
 * This function implements the rename operation, moving an inode from a source
 * path and name to a destination path and name. The operation is designed to be
 * atomic. If a file or directory already exists at the destination, it will be
 * overwritten and removed.
 *
 * @param srcpath   The path components leading to the source directory.
 * @param dstpath   The path components leading to the destination directory.
 * @param srcname   The name of the file or directory to rename.
 * @param dstname   The new name for the file or directory.
 *
 * @return 0 on success.
 * @return -1 on failure (e.g., path not found, source does not exist).
 *         On failure, the file system state remains unchanged.
 */
int atomfs_rename(char* srcpath[], char* dstpath[], char* srcname, char* dstname) {
    // Determine the common ancestor path for the source and destination.
    char** common_path = calculate(srcpath, dstpath);
    if (common_path == NULL) {
        return -1; // Memory allocation failure.
    }

    // --- Phase 1: Traverse the Common Path ---
    // Acquire a lock on the root and traverse down to the lowest common ancestor.
    lock(root_inum);
    // 'locate' uses lock-coupling: it locks the next inode before unlocking the
    // current one. Upon return, only the lock on 'parent' is held.
    struct inode* parent = locate(root_inum, common_path);
    if (parent == NULL) {
        // Traversal failed. 'locate' guarantees no locks are held on failure.
        free_path(common_path);
        return -1;
    }

    // --- Phase 2: Traverse Remaining Paths ---
    // Calculate the remaining path segments from the common ancestor.
    int common_len = getlen(common_path);
    char** src_remain = srcpath + common_len;
    char** dst_remain = dstpath + common_len;

    // From the common 'parent', find the source directory.
    // 'locate_hold' acquires locks on the traversal path but does NOT release
    // the lock on 'parent'. On success, both parent and srcdir locks are held.
    struct inode* srcdir = locate_hold(parent, src_remain);
    if (srcdir == NULL) {
        // Failed to find srcdir. Per the spec of 'locate_hold' on failure,
        // only the 'parent' lock is held. Release it and exit.
        unlock(parent);
        free_path(common_path);
        return -1;
    }

    // From the common 'parent', find the destination directory.
    // At this point, both 'parent' and 'srcdir' locks are held.
    struct inode* dstdir = locate_hold(parent, dst_remain);
    if (dstdir == NULL) {
        // Failed to find dstdir. Release the locks on 'parent' and 'srcdir'.
        // 'check_unlock' safely releases parent's lock, avoiding a double-unlock
        // if parent and srcdir are the same inode.
        check_unlock(parent, srcdir, NULL);
        unlock(srcdir);
        free_path(common_path);
        return -1;
    }

    // Now that both srcdir and dstdir are found and locked, release the lock on
    // the common ancestor, unless it's one of the directories we need.
    check_unlock(parent, srcdir, dstdir);
    free_path(common_path); // Free the memory for the common path array.

    // --- Phase 3: Perform Checks and Operations ---
    // Pre-condition: Locks on srcdir and dstdir are held.
    // This function checks for source existence and destination compatibility.
    // Per its spec, on failure, it is responsible for releasing all held locks.
    int check_result = check_src_exist_dst_delete(srcdir, dstdir, srcname, dstname);
    if (check_result == 1) {
        // All necessary locks were released by check_src_exist_dst_delete.
        return -1;
    }

    // Perform the atomic rename operation.
    // First, remove the source entry. This returns the inode being moved.
    struct inode* srcinode = inode_delete(srcdir, srcname);
    if (srcinode == NULL) {
        // This case should be prevented by the check_src_exist_dst_delete call.
        // It indicates an inconsistent state. Clean up and fail.
        unlock2dir(srcdir, dstdir);
        return -1;
    }

    // Remove the destination entry if it exists.
    struct inode* dstinode = inode_delete(dstdir, dstname);
    
    // Insert the source inode into the destination directory with the new name.
    int insert_result = inode_insert(dstdir, srcinode, dstname);
    if (insert_result != 0) {
        // Insertion failed. We must roll back to maintain atomicity.
        // Restore the source inode to its original directory.
        inode_insert(srcdir, srcinode, srcname);
        // If a destination inode was removed, restore it as well.
        if (dstinode != NULL) {
            inode_insert(dstdir, dstinode, dstname);
        }
        // Release locks and return failure.
        unlock2dir(srcdir, dstdir);
        return -1;
    }

    // The rename operation was successful. Clean up resources.
    // If a destination inode was overwritten, it must be disposed of.
    if (dstinode != NULL) {
        // Per the spec of check_src_exist_dst_delete, the lock on dstinode
        // is still held. It must be released before disposal.
        unlock(dstinode);
        dispose_inode(dstinode);
    }

    // Release the locks on the source and destination directories.
    unlock2dir(srcdir, dstdir);
    return 0; // Success
}