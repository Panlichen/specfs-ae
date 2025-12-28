#include "interface-util.h"

/**
 * @brief Checks pre-conditions for a rename-like operation.
 *
 * This function validates if a source entry can be "moved" to a destination.
 * It checks for the existence of the source, the validity of the destination
 * directory, and if the destination entry exists, whether it can be replaced.
 * It also manages the locking of the source and destination inodes according
 * to the specified protocol.
 *
 * @param srcdir Pointer to the locked source directory inode.
 * @param dstdir Pointer to the locked destination directory inode.
 * @param srcname The name of the source entry to check.
 * @param dstname The name of the destination entry.
 *
 * @return 0 on success, 1 on failure.
 */
int check_src_exist_dst_delete(struct inode *srcdir, struct inode *dstdir, char *srcname, char *dstname)
{
    // SPEC: Case 1.1 - Source Existence: An inode named `srcname` exists under `srcdir`.
    // Find the source inode within the source directory.
    struct inode *srcinode = inode_find(srcdir, srcname);
    if (srcinode == NULL) {
        // SPEC: Case 2 (Failure) - Source does not exist.
        // SPEC: Post-Condition on Failure: Release all locks and return.
        unlock2dir(srcdir, dstdir);
        return 1;
    }

    // SPEC: Case 1.2 - Destination Directory Validity.
    // Check if `dstdir` is a directory and has enough space for a new entry.
    if (dstdir->mode != DIR_MODE || dstdir->size >= MAX_DIR_SIZE) {
        // SPEC: Case 2 (Failure) - Destination directory is not a directory or is full.
        // SPEC: Post-Condition on Failure: Release all locks and return.
        unlock2dir(srcdir, dstdir);
        return 1;
    }

    // Check if an entry with `dstname` already exists in the destination directory.
    struct inode *dstinode = inode_find(dstdir, dstname);
    
    // Per the System Algorithm, we must lock the source inode before proceeding
    // with checks involving the destination inode.
    lock(srcinode);
    
    // This block handles SPEC: Case 1.3 - Destination Entry Validity.
    if (dstinode != NULL) {
        // The destination entry exists. We need to check if it can be replaced.

        // If the source and destination inodes are the same, it's a valid (though trivial) rename.
        // The checks below are only necessary if they are different inodes.
        if (srcinode != dstinode) {
            // Lock the destination inode to safely inspect its properties.
            lock(dstinode);
            
            // Check for type compatibility: both must be directories or both must not be.
            int src_is_dir = (srcinode->mode == DIR_MODE);
            int dst_is_dir = (dstinode->mode == DIR_MODE);
            
            if (src_is_dir != dst_is_dir) {
                // SPEC: Case 2 (Failure) - Incompatible types (e.g., renaming a file to a directory).
                // SPEC: Post-Condition on Failure: Release all locks and return.
                unlock(srcinode);
                unlock(dstinode);
                unlock2dir(srcdir, dstdir);
                return 1;
            }

            // If the destination is a directory, it must be empty to be replaced.
            if (dst_is_dir && dstinode->size != 0) {
                // SPEC: Case 2 (Failure) - Destination directory is not empty.
                // SPEC: Post-Condition on Failure: Release all locks and return.
                unlock(srcinode);
                unlock(dstinode);
                unlock2dir(srcdir, dstdir);
                return 1;
            }
            
            // SPEC: Post-Condition on Success: The lock on `srcinode` is released.
            // The lock on `dstinode` remains held for the caller to perform the deletion.
            unlock(srcinode);
        }
        // If srcinode == dstinode, we fall through. The srcinode lock (which is also the
        // dstinode lock) is not released, which is correct as the caller expects the
        // dstinode lock to be held on success.
    } else {
        // The destination entry does not exist, so no further checks are needed.
        // SPEC: Post-Condition on Success: The lock on `srcinode` is released.
        unlock(srcinode);
    }
    
    // SPEC: Case 1 (Success): All conditions are met. Return 0.
    // The locks on `srcdir`, `dstdir`, and `dstinode` (if it exists) are held as per the spec.
    return 0;
}