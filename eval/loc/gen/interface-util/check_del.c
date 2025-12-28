#include "interface-util.h"

/**
 * @brief Checks if a directory entry can be safely deleted.
 *
 * @param cur [in] A pointer to the parent directory inode.
 * @param name [in] The name of the entry to check for deletion.
 *
 *
 * @return 0 on success, indicating that deletion is permissible.
 * @return 1 on failure, indicating that deletion is not permissible.
 *
 */
int check_del(struct inode *cur, char *name) {
    // [SPEC] Case 2: `cur` is `NULL`.
    // Per the specification, this is a failure condition.
    if (cur == NULL) {
        return 1;
    }

    // [SPEC] Use `inode_find` to locate the entry `name` within the directory `cur`.
    struct inode *target = inode_find(cur, name);

    // [SPEC] Case 2: The entry `name` does not exist within `cur`.
    if (target == NULL) {
        // [SPEC] Post-Condition (Failure): Release all locks.
        // The caller held the lock for `cur`, so we must release it.
        unlock(cur);
        return 1;
    }

    // [SPEC] Post-Condition (Success): Acquire the lock for the target inode.
    // This lock is required to safely inspect the target's mode and size,
    // and must be held if the check succeeds.
    lock(target);

    // [SPEC] Case 1 (Deletion is permissible): The entry is either a file
    // (`mode != DIR_MODE`) or an empty directory (`size == 0`).
    if (target->mode != DIR_MODE || target->size == 0) {
        // The deletion is permissible. Return 0, leaving both `cur` and `target`
        // locked as per the success post-condition.
        return 0;
    }

    // [SPEC] Case 2: The entry `name` is a directory that is not empty.
    // If the check above fails, it means the target is a non-empty directory.
    // [SPEC] Post-Condition (Failure): Release all locks.
    // Both the acquired target lock and the pre-held current directory lock must be released.
    unlock(target);
    unlock(cur);
    return 1;
}