#include "path.h"

/**
 * @brief Traverses a file system path starting from a given directory inode.
 *
 *
 * @param cur A pointer to the starting directory inode.
 *            **Pre-Condition**: This inode must be a valid directory and must be
 *            **locked** by the caller before this function is invoked.
 * @param path A NULL-terminated array of strings, where each string is a
 *             component of the path to be traversed.
 *
 * @return On success, returns a pointer to the **locked** inode corresponding to the
 *         final component of the path.
 * @return If the path is empty (`path[0] == NULL`), it returns the original
 *         locked `cur` inode, fulfilling **Post-Condition Case 1**.
 * @return If any component in the path cannot be found, it returns `NULL`.
 *         In this failure case, all locks held by this function are released
 *         before returning, fulfilling **Post-Condition Case 3**.
 */
struct inode* locate(struct inode* cur, char* path[])
{
    // Step 1: Initialize 'current' with the starting inode 'cur'.
    // Per the pre-condition, 'cur' is already locked by the caller.
    struct inode* current = cur;

    // Handle Post-Condition Case 1: The path array is empty.
    // If there are no components to traverse, we simply return the starting
    // inode, which remains locked as required.
    if (path[0] == NULL) {
        return current;
    }

    // Step 2: Begin the iterative traversal loop for each path component.
    for (int i = 0; path[i] != NULL; i++) {
        // Step 2.a: Find the next inode corresponding to the current path
        // component without acquiring any locks yet.
        struct inode* next = inode_find(current, path[i]);

        // Step 2.b: Handle the path failure case (Component missing).
        if (next == NULL) {
            // A component was not found in the directory 'current'.
            // To meet the failure post-condition, release the lock on the
            // 'current' inode before returning.
            unlock(current);
            // Return NULL to signal that the path does not exist.
            return NULL;
        }

        // Step 2.c: Handle path success for this component using lock coupling.
        // First, acquire the lock on the child/next inode.
        lock(next);
        // Only after successfully locking the next inode, release the lock on the
        // parent/current inode. This ensures a continuous locked path.
        unlock(current);

        // Advance the pointer to move one level down the directory tree for the
        // next iteration. 'current' is now the locked 'next' inode.
        current = next;
    }

    // Step 3: Successful termination. The loop has completed.
    // 'current' now points to the inode of the final path component.
    // Its lock was acquired during the last loop iteration and was not released,
    // satisfying the success post-condition (Case 2).
    return current;
}