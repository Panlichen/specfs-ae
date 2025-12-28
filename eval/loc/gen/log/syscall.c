/**
 * @brief Renames a file or directory, treating the operation as a transaction.
 *
 * The destination inode, if it exists, is not immediately disposed of but is
 * logged for deferred deletion.
 */
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

    begin_trans(WRITE_MODE);

    compath = calculate(srcpath, dstpath);
    lock( root_inum );
    parent = locate(root_inum, compath);
    if( parent == NULL )
    {
        free_path(compath);
        end_trans(WRITE_MODE);
        return -1;
    }
    comlen = getlen(compath);
    free_path(compath);
    srcdir = locate_hold(parent , srcpath + comlen);
    if( srcdir == NULL )
    {
        unlock( parent );
        end_trans(WRITE_MODE);
        return -1;
    }
    dstdir = locate_hold(parent , dstpath + comlen );
    if ( dstdir == NULL )
    {
        check_unlock(parent, srcdir, NULL);
        unlock ( srcdir );
        end_trans(WRITE_MODE);
        return -1;
    }
    check_unlock(parent, srcdir, dstdir );
    temp = check_src_exist_dst_delete(srcdir, dstdir, srcname, dstname );
    if ( temp )
    {
        unlock2dir(srcdir, dstdir); // Assume locks are released on error
        end_trans(WRITE_MODE);
        return -1;
    }
    srcinode = delete ( srcdir, srcname );
    dstinode = delete ( dstdir, dstname );
    insert ( dstdir, srcinode , dstname  );
    unlock2dir(srcdir, dstdir );

    if ( dstinode != NULL )
        log_delete(dstinode, LOG_TYPE_INODE); // Log inode for deferred deletion

    end_trans(WRITE_MODE);
    return 0;
}

/**
 * @brief Opens an inode, treating the operation as a transaction.
 */
struct inode* atomfs_open(char* path[], unsigned mode)
{
    struct inode* inum;
    int temp;

    begin_trans(READ_MODE);

    lock( root_inum );
    inum = locate(root_inum, path);
    temp = check_open(inum, mode);
    if (temp == 1) {
        // Assume lock on inum is released if it was acquired
        if (inum) unlock(inum);
        end_trans(READ_MODE);
        return NULL;
    }
    
    // The transaction for the filesystem read is complete.
    end_trans(READ_MODE);
    return inum;
}

/**
 * @brief Inserts a new inode, treating the operation as a transaction.
 */
int atomfs_ins(char* path[], char* name, int mode, unsigned maj, unsigned min)
{
    struct inode* cur;
    struct inode* inum;
    unsigned temp;

    begin_trans(WRITE_MODE);

    lock(root_inum);
    cur = locate(root_inum, path);
    temp = check_ins(cur , name);
    if(temp)
    {
        if (cur) unlock(cur); // Assume lock is released on error
        end_trans(WRITE_MODE);
        return -1;
    }
    inum = malloc_inode(mode, maj, min);
    insert(cur,inum, name );
    unlock(cur);

    end_trans(WRITE_MODE);
    return 0;
}

/**
 * @brief Deletes an inode, treating the operation as a transaction.
 *
 * The inode is not immediately disposed of but is logged for deferred deletion.
 */
int atomfs_del(char* path[], char* name)
{
    struct inode* cur;
    struct inode* inum;
    unsigned temp;

    begin_trans(WRITE_MODE);

    lock( root_inum );
    cur = locate(root_inum, path);
    temp = check_del(cur, name);
    if( temp )
    {
        if (cur) unlock(cur); // Assume lock is released on error
        end_trans(WRITE_MODE);
        return -1;
    }
    inum = delete(cur,name);
    unlock( cur );

    log_delete(inum, LOG_TYPE_INODE); // Log inode for deferred deletion

    end_trans(WRITE_MODE);
    return 0;
}

/**
 * @brief Reads directory entries, treating the operation as a transaction.
 */
char** atomfs_readdir(char* path[])
{
    struct inode* inum;
    char** dircontent;
    int temp;

    begin_trans(READ_MODE);

    lock( root_inum );
    inum = locate(root_inum, path);
    temp = check_dir(inum);
    if (temp == 1)
    {
        if (inum) unlock(inum); // Assume lock is released on error
        end_trans(READ_MODE);
        return NULL;
    }
    dircontent = malloc_dir_content(inum->size + 1);
    filldir(inum, dircontent);
    unlock(inum);

    end_trans(READ_MODE);
    return dircontent;
}

/**
 * @brief Gets inode attributes, treating the operation as a transaction.
 */
struct getattr_ret* atomfs_getattr(char* path[])
{
    struct inode* inum;
    struct getattr_ret* ret;

    begin_trans(READ_MODE);

    lock( root_inum );
    inum = locate(root_inum, path);
    if(inum == NULL)
    {
        end_trans(READ_MODE);
        return NULL;
    }
    ret = malloc_getattr_ret(inum, inum->mode, inum->size, inum->maj, inum->min);
    unlock(inum);

    end_trans(READ_MODE);
    return ret;
}