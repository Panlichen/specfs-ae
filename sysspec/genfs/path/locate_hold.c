#include "path.h"

struct inode* locate_hold(struct inode *cur, char *path[]) {
    // Empty path: cur is the target, lock is already held
    if (path[0] == NULL) {
        return cur;
    }
    
    // Find first component in cur's directory
    struct inode* next_inum = inode_find(cur, path[0]);
    if (next_inum == NULL) {
        // Component not found, retain lock on cur and return NULL
        return NULL;
    }
    
    // Lock the child inode while still holding lock on cur
    lock(next_inum);
    
    // Delegate remaining path traversal to locate
    // locate will release the lock on next_inum (and any intermediate locks)
    // and return the final target (locked) or NULL (all locks released)
    // Our lock on cur remains held throughout
    return locate(next_inum, path + 1);
}