// main_dir.h functions

int atomfs_ins(char* path[], char* name, int mode, unsigned maj, unsigned min)
{
     struct inode* cur;
     struct inode* inum;
     unsigned temp;

     lock(root_inum);
     cur = locate(root_inum, path);
     temp = check_ins(cur , name);
     if(temp)
         return -1;
     inum = malloc_inode(mode, maj, min);
     // Set all timestamps for the new file
     set_current_time(inum, ACCESS_TIME);
     set_current_time(inum, MODIFICATION_TIME);
     set_current_time(inum, CHANGE_TIME);

     insert(cur,inum, name );
     // Set access and modification times for the parent directory
     set_current_time(cur, ACCESS_TIME);
     set_current_time(cur, MODIFICATION_TIME);

     unlock(cur);
     return 0;
}


int atomfs_del(char* path[], char* name)
{
     struct inode* cur;
     struct inode* inum;
     unsigned temp;
     lock( root_inum );
     cur = locate(root_inum, path);
     temp = check_del(cur, name);
     if( temp )
         return -1;
     inum = delete(cur,name);
     // Set access and modification times for the parent directory
     set_current_time(cur, ACCESS_TIME);
     set_current_time(cur, MODIFICATION_TIME);

     unlock( cur );
     dispose_inode( inum );
     return 0;
}

char** atomfs_readdir(char* path[])
{
     struct inode* inum;
     char** dircontent;
     int temp;
     lock( root_inum );
     inum = locate(root_inum, path);
     temp = check_dir(inum);
     if (temp == 1)
         return NULL;
     // Set the directory's access time
     set_current_time(inum, ACCESS_TIME);
     dircontent = malloc_dir_content(inum->size + 1);
     filldir(inum, dircontent);
     unlock(inum);
     return dircontent;
}

struct getattr_ret* atomfs_getattr(char* path[])
{
     struct inode* inum;
     struct getattr_ret* ret;
     lock( root_inum );
     inum = locate(root_inum, path);
     if(inum == NULL)
         return NULL;
     // Set the file's access time
     set_current_time(inum, ACCESS_TIME);
     ret = malloc_getattr_ret(inum, inum->mode, inum->size, inum->maj, inum->min);
     unlock(inum);
     return ret;
}

// main_file.h functions

struct read_ret* atomfs_read(char* path[], unsigned size, unsigned offset)
{
    struct inode* inum;
    struct read_ret* ret;
    int temp;
    lock(root_inum);
    inum = locate(root_inum, path);
    temp = check_file(inum);
    if (temp == 1)
        return NULL;
    // Set the file's access time
    set_current_time(inum, ACCESS_TIME);
    ret = inode_read(inum, size, offset);
    unlock(inum);
    return ret;
}

int atomfs_write(char* path[], const char* buf, unsigned size, unsigned offset)
{
    struct inode* inum;
    int ret;
    lock(root_inum);
    inum = locate(root_inum, path);
    ret = check_file(inum);
    if (ret == 1)
        return -1;
    // Set the file's access and modification times
    set_current_time(inum, ACCESS_TIME);
    set_current_time(inum, MODIFICATION_TIME);
    ret = inode_write(inum, buf, size, offset);
    unlock(inum);
    return ret;
}

int atomfs_truncate(char* path[], unsigned offset)
{
    struct inode* inum;
    int ret;
    if( offset > MAX_FILE_SIZE)
        return -1;
    lock(root_inum);
    inum = locate(root_inum, path);
    ret = check_file(inum);
    if (ret == 1)
        return -1;
    // Set the file's access and modification times
    set_current_time(inum, ACCESS_TIME);
    set_current_time(inum, MODIFICATION_TIME);
    inode_truncate(inum, offset);
    unlock(inum);
    return 0;
}

struct inode* atomfs_open(char* path[], unsigned mode)
{
    struct inode* inum;
    int temp;
    lock( root_inum );
    inum = locate(root_inum, path);
    temp = check_open(inum, mode);
    if (temp == 1)
        return NULL;
    // Set the file's access time
    set_current_time(inum, ACCESS_TIME);
    return inum;
}

// main_rename.h function

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

    // Set timestamps for the moved file
    set_current_time(srcinode, ACCESS_TIME);
    set_current_time(srcinode, MODIFICATION_TIME);
    
    // Set timestamps for the source directory
    set_current_time(srcdir, ACCESS_TIME);
    set_current_time(srcdir, MODIFICATION_TIME);

    // Set timestamps for the destination directory
    set_current_time(dstdir, ACCESS_TIME);
    set_current_time(dstdir, MODIFICATION_TIME);

    unlock2dir(srcdir, dstdir );
    if ( dstinode != NULL )
        dispose_inode( dstinode );
    return 0;
}