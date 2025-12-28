/*
 * [Module Name] Main_File
 * [Rely] Path_Handling, Inode_management and Commons
 */

#include "main_file.h"


int check_file(struct inode* inum)
{
    if( inum == NULL )
        return 1;
    if( inum->mode == DIR_MODE )
    {
        unlock( inum );
        return 1;
    }
    return 0;
}

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
    inode_truncate(inum, offset);
    unlock(inum);
    return 0;
}



int check_open(struct inode* inum, unsigned mode)
{
    if(inum == NULL)
        return 1;
    if(inum->mode == DIR_MODE && mode != DIR_MODE)
    {
        unlock(inum);
        return 1;
    }
    if(inum->mode != DIR_MODE && mode == DIR_MODE)
    {
        unlock(inum);
        return 1;
    }
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
    return inum;
}

