#include "interface.h"

int atomfs_truncate(char* path[], unsigned offset)
{
    if (offset > MAX_FILE_SIZE) {
        return -1;
    }

    // Lock root directory
    lock(root_inum);
    // Locate the target inode
    struct inode *target = locate(root_inum, path);
    if (target == NULL) {
        return -1;
    }

    // Check if it's a regular file
    if (check_file(target) != 0) {
        return -1;
    }

    // Truncate the file
    inode_truncate(target, offset);
    unlock(target);
    return 0;
}