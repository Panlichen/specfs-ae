#include "interface-util.h"

/**
 * @brief Checks if a new entry can be inserted into a directory inode.
 *
 * @param cur A pointer to the directory inode where the insertion is proposed.
 * @param name A valid, non-NULL string for the new entry's name.
 * 
 * @pre The lock for the inode `cur` must be held by the calling thread.
 * 
 * @return 0 on success, indicating the insertion is possible. The lock on `cur` remains held.
 * @return 1 on failure, indicating the insertion is not possible. The lock on `cur` is released
 *           before returning.
 */
int check_ins(struct inode *cur, char *name) {
    // [SPEC] Case 2: Failure if `cur` is `NULL`.
    // No lock can be released as the pointer is invalid.
    if (cur == NULL) {
        return 1;
    }

    // [SPEC] Case 2: Failure if `cur` is not a directory.
    if (cur->mode != DIR_MODE) {
        // [Refine Spec] Release the lock on failure before returning.
        unlock(cur);
        return 1;
    }

    // [SPEC] Case 2: Failure if the directory is already full.
    if (cur->size >= MAX_DIR_SIZE) {
        // [Refine Spec] Release the lock on failure before returning.
        unlock(cur);
        return 1;
    }

    // [SPEC] Case 2: Failure if an entry with the given `name` already exists.
    // Use the provided `inode_find` helper to perform the check.
    if (inode_find(cur, name) != NULL) {
        // [Refine Spec] Release the lock on failure before returning.
        unlock(cur);
        return 1;
    }
    
    // [SPEC] Case 1: Success. All pre-conditions for insertion are met.
    // [Refine Spec] The lock on `cur` remains held by the caller.
    return 0;
}