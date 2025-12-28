/*
 * [Module Name] Main_Rename
 * [Rely] Path_Handling, Directory_OPs, Inode_management and Commons
 */

#include "main_rename.h"

struct inode* locate_hold(struct inode* cur, char* path[])
{
    int i;
    struct inode* next_inum;
    struct inode* ret;
    i = 0;
    if( path[i] == NULL )
        return cur;
    next_inum = find (cur->dir, path [i]);
    if ( next_inum == NULL )
        return NULL;
    lock( next_inum );
    ret = locate(next_inum , path + 1);
    return ret;
}

void check_unlock (struct inode* parent, struct inode* src, struct inode* dst)
{
    if ( parent != src && parent != dst)
        unlock ( parent );
    return;
}

void unlock2dir (struct inode* srcdir, struct inode* dstdir)
{
    if( srcdir != dstdir )
    {
        unlock ( srcdir );
        unlock ( dstdir );
    }
    else
        unlock ( srcdir );
    return;
}

int check_src_exist_dst_delete(struct inode* srcdir, struct inode* dstdir, char* srcname, char* dstname)
{
    struct inode* srcinode;
    struct inode* dstinode;
    srcinode = find(srcdir->dir, srcname);
    if ( srcinode == NULL )
    {
        unlock2dir(srcdir, dstdir);
        return 1;
    }

    if(dstdir->mode != DIR_MODE)
    {
        unlock2dir(srcdir, dstdir);
        return 1;
    }
    if(dstdir->size >= MAX_DIR_SIZE)
    {
        unlock2dir(srcdir, dstdir);
        return 1;
    }
    dstinode = find( dstdir->dir, dstname );
    if(srcinode == dstinode)
        return 0;
    if( dstinode != NULL )
    {
        lock ( srcinode );
        lock ( dstinode );
        if(srcinode->mode == DIR_MODE && dstinode->mode != DIR_MODE)
        {
            unlock(srcinode);
            unlock( dstinode );
            unlock2dir(srcdir, dstdir);
            return 1;
        }
        if(srcinode->mode != DIR_MODE && dstinode->mode == DIR_MODE)
        {
            unlock(srcinode);
            unlock( dstinode );
            unlock2dir(srcdir, dstdir);
            return 1;
        }
        if( dstinode->mode == DIR_MODE && dstinode->size > 0)
        {
            unlock(srcinode);
            unlock( dstinode );
            unlock2dir(srcdir, dstdir);
            return 1;
        }
        unlock( srcinode );
    }
    return 0;
}

int atomfs_rename(char* srcpath[], char* dstpath[], char* srcname, char* dstname)
{
    char** compath;
    int comlen;
    struct inode* parent;
    struct inode* srcdir;
    struct inode* dstdir;
    struct inode* srcinode;
    struct inode* dstinode;
    unsigned temp;
    
    compath = calculate(srcpath, dstpath);
    lock( root_inum );
    parent = locate(root_inum, compath);
    if( parent == NULL )
    {
        free_path(compath);
        return -1;
    }
    comlen = getlen(compath);
    free_path(compath);
    srcdir = locate_hold(parent , srcpath + comlen);
    if( srcdir == NULL )
    {
        unlock( parent );
        return -1;
    }
    dstdir = locate_hold(parent , dstpath + comlen );
    if ( dstdir == NULL )
    {
        check_unlock(parent, srcdir, NULL);
        unlock ( srcdir );
        return -1;
    }
    check_unlock(parent, srcdir, dstdir );
    temp = check_src_exist_dst_delete(srcdir, dstdir, srcname, dstname );
    if ( temp )
        return -1;
    srcinode = delete ( srcdir, srcname );
    dstinode = delete ( dstdir, dstname );
    insert ( dstdir, srcinode , dstname  );
    unlock2dir(srcdir, dstdir );
    if ( dstinode != NULL )
        dispose_inode( dstinode );
    return 0;
}

