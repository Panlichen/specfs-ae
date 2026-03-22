#include "interface.h"

struct read_ret* atomfs_read(char* path[], unsigned size, unsigned offset) {
    // Acquire root_inum lock
    lock(root_inum);
    
    // Locate target inode
    struct inode* target = locate(root_inum, path);
    
    // If not found, all locks are already released by locate
    if (target == NULL) {
        return NULL;
    }
    
    // Check if it's a readable file
    int check = check_file(target);
    if (check == 1) {
        // check_file releases lock on failure
        return NULL;
    }
    
    // target is still locked at this point
    struct read_ret* result = inode_read(target, size, offset);
    
    // Release target lock before returning
    unlock(target);
    
    return result;
}