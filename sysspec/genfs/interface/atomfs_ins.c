#include "interface.h"

int atomfs_ins(char* path[], char* name, int mode, unsigned maj, unsigned min)
{
    struct inode *target = NULL;
    struct inode *new_inode = NULL;
    int result = -1;

    // Lock the root to start traversal
    lock(root_inum);
    
    // Locate the target directory
    target = locate(root_inum, path);
    
    // If target is found, it is locked. If not found, no locks are held.
    if (target == NULL) {
        // locate already released all locks, just return failure
        return -1;
    }

    // Check if insertion is allowed (target is locked)
    if (check_ins(target, name) != 0) {
        // check_ins released the lock on failure
        return -1;
    }
    // After successful check_ins, target remains locked

    // Allocate new inode
    new_inode = malloc_inode(mode, maj, min);
    if (new_inode == NULL) {
        unlock(target);
        return -1;
    }

    // Insert the new entry into target directory
    if (inode_insert(target, new_inode, name) != 0) {
        unlock(target);
        return -1;
    }

    // Release lock on target directory
    unlock(target);

    // Success
    return 0;
}