#include "interface.h"

struct getattr_ret* atomfs_getattr(char* path[]) {
    /* Lock the root inode before calling locate */
    lock(root_inum);
    
    /* Traverse the path starting from root_inum */
    struct inode* target = locate(root_inum, path);
    
    /* If target is NULL, path doesn't exist */
    if (target == NULL) {
        /* No lock is owned when locate returns NULL */
        return NULL;
    }
    
    /* Target is found and locked (by locate) */
    /* Extract attributes from the target inode */
    unsigned mode = target->mode;
    unsigned size = target->size;
    unsigned maj = target->maj;
    unsigned min = target->min;
    
    /* Unlock the target inode */
    unlock(target);
    
    /* Allocate and populate the return structure */
    struct getattr_ret* ret = malloc_getattr_ret(target, mode, size, maj, min);
    
    /* Post-condition: no lock is owned */
    return ret;
}