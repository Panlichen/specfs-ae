#include "interface.h"

int atomfs_del(char* path[], char* name) {
    lock(root_inum);
    struct inode* parent = locate(root_inum, path);
    if (parent == NULL) {
        return -1;
    }
    
    int allowed = check_del(parent, name);
    if (allowed != 0) {
        return -1;
    }
    
    struct inode* child = inode_delete(parent, name);
    if (child == NULL) {
        unlock(parent);
        return -1;
    }
    
    unlock(parent);
    unlock(child);
    dispose_inode(child);
    return 0;
}