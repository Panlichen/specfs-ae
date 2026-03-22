#include "interface.h"

int atomfs_write(char* path[], const char* buf, unsigned size, unsigned offset) {
    // Pre-condition: no lock is owned
    lock(root_inum);  // Lock root_inum before calling locate
    struct inode* target = locate(root_inum, path);  // locate will release root_inum lock
    
    if (target == NULL) {
        // Post-condition: no lock is owned (locate already released all locks)
        return -1;
    }
    
    // target is locked by locate
    int check = check_file(target);
    if (check == 1) {
        // check_file releases target lock when returning 1
        // Post-condition: no lock is owned
        return -1;
    }
    
    // target is still locked by check_file (when returning 0)
    unsigned result = inode_write(target, buf, size, offset);
    unlock(target);  // Release target lock after write
    
    // Post-condition: no lock is owned
    return result;
}