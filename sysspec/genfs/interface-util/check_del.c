#include "interface-util.h"

int check_del(struct inode *cur, char *name) {
    // Pre-condition: lock for cur is held
    
    struct inode *target = inode_find(cur, name);
    if (target == NULL) {
        // Entry doesn't exist, release cur's lock and return failure
        unlock(cur);
        return 1;
    }
    
    // Lock the target inode
    lock(target);
    
    // Check if target is a non-empty directory
    if (target->mode == DIR_MODE && target->size > 0) {
        // Non-empty directory, release both locks and return failure
        unlock(cur);
        unlock(target);
        return 1;
    }
    
    // Success: both locks are held (cur's lock was never released,
    // target's lock is held, and we return 0)
    return 0;
}