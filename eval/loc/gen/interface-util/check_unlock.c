#include "interface-util.h"

/**
 * @brief Conditionally unlocks a parent inode.
 *
 * Implements the logic to unlock a `parent` inode only if it is distinct from
 * both a `src` (source) and `dst` (destination) inode. This is typically used
 * at the end of a file system operation to correctly manage locks and prevent
 * unlocking the same inode twice if the parent is also the source or destination.
 *
 * @param parent Pointer to the parent inode, which is currently locked.
 * @param src    Pointer to the source inode.
 * @param dst    Pointer to the destination inode.
 *
 */

void check_unlock(struct inode* parent, struct inode* src, struct inode* dst)
{
    // Per the specification, unlock the parent only if it is a distinct inode
    // from both the source and destination. This check is performed by comparing
    // the inode pointers.
    if (parent != src && parent != dst) {
        // Case 1: The parent is distinct, so we perform the unlock by calling
        // the `unlock` function provided in the [RELY] condition.
        unlock(parent);
    }
    // Case 2: If the parent pointer matches either src or dst, the condition
    // is false, and no operation occurs, satisfying the specification.
}