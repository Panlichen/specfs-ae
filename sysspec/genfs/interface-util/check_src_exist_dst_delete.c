#include "interface-util.h"

int check_src_exist_dst_delete(struct inode *srcdir, struct inode *dstdir, char *srcname, char *dstname) {
    struct inode *srcinode = NULL;
    struct inode *dstinode = NULL;
    int src_locked = 0;
    int dst_locked = 0;

    // Check source existence
    srcinode = inode_find(srcdir, srcname);
    if (srcinode == NULL) {
        unlock2dir(srcdir, dstdir);
        return 1;
    }

    // Check destination directory validity
    if (dstdir->mode != DIR_MODE || dstdir->size >= MAX_DIR_SIZE) {
        unlock2dir(srcdir, dstdir);
        return 1;
    }

    // Check destination entry existence
    dstinode = inode_find(dstdir, dstname);
    
    if (dstinode != NULL) {
        // Lock srcinode first
        lock(srcinode);
        src_locked = 1;
        
        // Lock dstinode only if different from srcinode
        if (dstinode != srcinode) {
            lock(dstinode);
            dst_locked = 1;
            
            // Check type compatibility
            int src_is_dir = (srcinode->mode == DIR_MODE);
            int dst_is_dir = (dstinode->mode == DIR_MODE);
            
            if (src_is_dir != dst_is_dir) {
                // Release all acquired locks
                if (dst_locked) unlock(dstinode);
                unlock(srcinode);
                unlock2dir(srcdir, dstdir);
                return 1;
            }
            
            // Check if destination directory is empty
            if (dst_is_dir && dstinode->size != 0) {
                if (dst_locked) unlock(dstinode);
                unlock(srcinode);
                unlock2dir(srcdir, dstdir);
                return 1;
            }
            
            // Success case for different inodes: unlock srcinode, keep dstinode locked
            unlock(srcinode);
        } else {
            // Same inode case: keep it locked (as dstinode), no additional checks needed
        }
    }
    
    return 0;
}