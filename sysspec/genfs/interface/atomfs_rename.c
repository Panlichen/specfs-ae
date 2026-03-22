#include "interface.h"

int atomfs_rename(char* srcpath[], char* dstpath[], char* srcname, char* dstname) {
    // Phase 1: Traverse to common parent
    lock(root_inum);
    char** common = calculate(srcpath, dstpath);
    struct inode* parent = locate(root_inum, common);
    free_path(common);
    if (parent == NULL) {
        // locate already released root_inum lock on failure
        return -1;
    }

    // Phase 2: Traverse remaining paths
    int src_len = getlen(srcpath);
    int dst_len = getlen(dstpath);
    char** src_remain = srcpath + getlen(common);
    char** dst_remain = dstpath + getlen(common);
    
    struct inode* srcdir = locate_hold(parent, src_remain);
    if (srcdir == NULL) {
        unlock(parent);
        return -1;
    }
    
    struct inode* dstdir = locate_hold(parent, dst_remain);
    if (dstdir == NULL) {
        check_unlock(parent, srcdir, dstdir);
        unlock(srcdir);
        return -1;
    }
    
    check_unlock(parent, srcdir, dstdir);

    // Phase 3: Validate and perform rename
    if (check_src_exist_dst_delete(srcdir, dstdir, srcname, dstname)) {
        return -1;  // Function releases all locks on failure
    }
    
    struct inode* srcinode = inode_delete(srcdir, srcname);
    if (srcinode == NULL) {
        unlock2dir(srcdir, dstdir);
        return -1;
    }
    
    struct inode* dstinode = inode_delete(dstdir, dstname);
    if (dstinode != NULL) {
        dispose_inode(dstinode);
    }
    
    if (inode_insert(dstdir, srcinode, dstname)) {
        unlock2dir(srcdir, dstdir);
        return -1;
    }
    
    unlock2dir(srcdir, dstdir);
    return 0;
}