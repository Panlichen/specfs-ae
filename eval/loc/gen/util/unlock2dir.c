#include "util.h"

/**
 * @brief Releases the locks on two directory inodes.
 *
 *
 * @param srcdir A pointer to the source directory inode to be unlocked.
 * @param dstdir A pointer to the destination directory inode to be unlocked.
 */
void unlock2dir(struct inode* srcdir, struct inode* dstdir) {
    // Check if the source and destination directory pointers refer to the same inode.
    if (srcdir == dstdir) {
        // If they are the same, unlock the inode only once.
        unlock(srcdir);
    } else {
        // If they are different inodes, unlock both of them. The order of
        // unlocking does not matter.
        unlock(srcdir);
        unlock(dstdir);
    }
}