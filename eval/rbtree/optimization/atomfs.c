/* * Copyright (c) 2020 Institution of Parallel and Distributed System, Shanghai Jiao Tong University
 * AtomFS is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include "common.h"
#include "atomfs.h"
#include "util.h"
#include <assert.h>
#include <stdlib.h>
#include "hashing.h"
#include "lowlevel_file.h"
#include "inode_management.h"
#include "path_handling.h"


void insert(struct inode* cur, struct inode* inum, char* name)
{
    struct entry* new_entry;
    unsigned hashval;

    hashval = hash_func(name);
    new_entry = malloc_entry();
    new_entry->name = malloc_string(name);
    new_entry->inum = inum;
    new_entry->next = cur->dir->tb[hashval];
    cur->dir->tb[hashval] = new_entry;
    cur->size = cur->size + 1;
    return;
}

void free_entry(struct entry* en)
{
    free(en->name);
    free(en);
    return;
}

struct inode* delete(struct inode* inum, char* name)
{
    struct entry* walk;
    struct entry* next_walk;
    struct inode* ret;
    struct entry** hd;
    struct entry* cur;
    unsigned int hashval;
    unsigned cmp;

    if(inum->dir == NULL)
        return NULL;
    hashval = hash_func(name);
    hd = &(inum->dir->tb[hashval]);
    cur = *hd;
    if(cur == NULL)
        return NULL;
    cmp = strcmp(cur->name, name);
    if(cmp == 0)
    {
        ret = cur->inum;
        *hd = cur->next;
        free_entry(cur);
        inum->size = inum->size - 1;
        return ret;
    }
    walk = cur;
    next_walk = walk->next;
    while(next_walk != NULL)
    {
        cmp = strcmp(next_walk->name, name);
        if(cmp==0)
        {
            walk->next = next_walk->next;
            ret = next_walk->inum;
            free_entry(next_walk);
    		inum->size = inum->size - 1;
            return ret;
        }
        walk = next_walk;
        next_walk = next_walk->next;
    }
    return NULL;
}


// struct inode* find(struct dirtb* dir, char* name)
// {
//     struct entry* walk;
//     unsigned int hash_val;
//     unsigned temp;

//     if(dir == NULL)
//         return NULL;
//     hash_val = hash_func(name);
//     walk = dir->tb[hash_val];
//     while(walk != NULL)
//     {
//         temp = strcmp(walk->name, name);
//         if(temp==0)
//             return walk->inum;
//         walk = walk->next;
//     }
//     return NULL;
// }

// struct inode* locate(struct inode* cur, char* path[])
// {
//     int i;
//     struct inode* next_inum;
//     i = 0;
//     while (path [i] != NULL)
//     {
//         next_inum = find (cur->dir, path [i]);
//         if ( next_inum == NULL )
//         {
//             unlock (cur );
//             return NULL;
//         }
//         lock( next_inum );
//         unlock(cur);
//         cur = next_inum;
//         i = i + 1;
//     }
//     return cur;
// }

int check_ins(struct inode *cur , char* name)
{
    struct inode* inum;
    if ( cur == NULL )
        return 1;
    if( cur->mode != DIR_MODE )
    {
        unlock(cur);
        return 1;
    }
    if(cur->size >= MAX_DIR_SIZE)
    {
        unlock(cur);
        return 1;
    }
    inum = find (cur->dir, name);
    if( inum != NULL )
    {
        unlock(cur);
        return 1;
    }
    return 0;
}

int atomfs_ins(char* path[], char* name, int mode, unsigned maj, unsigned min)
{
    struct inode* cur;
    struct inode* inum;
    unsigned temp;

    lock(root_inum);
    cur = locate(root_inum, path);
    temp = check_ins(cur , name );
    if(temp)
        return -1;
    inum = malloc_inode(mode, maj, min);
    insert(cur,inum, name );
    unlock(cur);
    return 0;
}

int check_del(struct inode* cur, char* name)
{
    struct inode* inum ;
    if( cur == NULL )
        return 1;
    inum = find(cur->dir , name);
    if( inum == NULL )
    {
        unlock( cur );
        return 1;
    }
    lock( inum );
    if((inum->mode == DIR_MODE) && (inum->size > 0))
    {
        unlock( inum );
        unlock( cur );
        return 1;
    }
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
    unlock( cur );
    dispose_inode( inum );
    return 0;
}

// char** calculate(char* srcpath[], char* dstpath[])
// {
//     int i;
//     char** compath;
//     unsigned temp;
//     i = 0;
//     while(srcpath[i]!=NULL)
//     {
//         i = i + 1;
//     }
//     compath = malloc_path( i + 1 );
//     i = 0;
//     while(srcpath[i]!=NULL && dstpath[i]!=NULL)
//     {
//         temp = strcmp(srcpath[i],dstpath[i]);
//         if(temp != 0)
//             return compath;
//         compath[i] = malloc_string(srcpath[i]);
//         i = i + 1;
//     }
//     return compath;
// }

// int getlen(char* path[])
// {
//     int i;
//     i = 0;
//     while(path[i]!=NULL)
//         i = i + 1;
//     return i;
// }

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

int check_dir(struct inode* inum)
{
    if ( inum == NULL )
         return 1;
    if( inum->mode != DIR_MODE )
    {
        unlock( inum );
        return 1;
    }
    return 0;
}

// void file_read(struct indextb *tb, unsigned offset, unsigned len, char *data)
// {
//     unsigned index;
//     unsigned off;
//     unsigned sz;
//     unsigned char *temp;
//     index = offset >> 12;
//     off = offset & 0xfff;

//     while (len) {
//         temp = tb->index[index];
//         sz = PG_SIZE - off;
//         if (len < sz)
//             sz = len;
//         memcpy(data, temp + off, sz);
//         len -= sz;
//         data += sz;
//         index = index + 1;
//         off = 0;
//     }

//     return;
// }

// static struct read_ret* inode_read(struct inode* node, unsigned len, unsigned offset)
// {
//     unsigned file_size;
//     unsigned actual_len;
//     struct read_ret* ret;

//     ret = malloc_readret();
//     file_size = node->size;
//     if (offset >= file_size)
//         return ret;
//     if ((offset + len) <= file_size)
//         actual_len = len;
//     else
//         actual_len = file_size - offset;
//     ret->buf = malloc_buffer( actual_len );
//     ret->num = actual_len;
//     file_read(node->file, offset, actual_len, ret->buf);
//     return ret;
// }

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

// void file_write(struct indextb *tb, unsigned offset, unsigned len,
//         const char *data)
// {
//     unsigned index;
//     unsigned off;
//     unsigned sz;
//     unsigned char *temp;
//     index = offset >> 12;
//     off = offset & 0xfff;

//     while (len) {
//         temp = tb->index[index];
//         sz = PG_SIZE - off;
//         if (len < sz)
//             sz = len;
//         if (data == NULL)
//             memset(temp + off, 0, sz);
//         else
//             memcpy(temp + off, data, sz);
//         len -= sz;
//         if (data)
//             data += sz;
//         index = index + 1;
//         off = 0;
//     }

//     return;
// }

// void file_allocate(struct indextb *tb, unsigned offset, unsigned len)
// {
//     unsigned char *p;
//     unsigned i;
//     unsigned end_i;

//     if(len == 0)
//         return;
//     i = offset >> 12;
//     end_i = (offset + len - 1) >> 12;
//     while (i <= end_i) {
//         if (tb->index[i] == NULL) {
//             p = malloc_page();
//             tb->index[i] = p;
//         }
//         i = i +1;
//     }
//     return;
// }

// unsigned inode_write(struct inode *node, const char* buffer, unsigned len, unsigned offset)
// {
//     unsigned file_size;

//     file_size = node->size;
//     if ( offset + len > MAX_FILE_SIZE )
//         len = MAX_FILE_SIZE - offset;

//     if (offset > file_size) {
//         file_allocate(node->file, file_size,
//                   offset + len - file_size);
//         file_write(node->file, file_size,
//                offset - file_size, NULL);
//     } else
//         file_allocate(node->file, offset, len);
//     file_write(node->file, offset, len, buffer);

//     if(offset + len > file_size)
//         node->size = offset + len;

//     return len;
// }

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

// void clear_file(struct inode *node, unsigned start, unsigned len)
// {
//     file_allocate(node->file, start, len);
//     file_write(node->file, start, len, NULL);
//     return;
// }

// void inode_truncate(struct inode *node, unsigned size)
// {
//     unsigned temp;
//     unsigned file_size;

//     file_size = node->size;
//     if (size > file_size) {
//         temp = size - file_size;
//         clear_file(node, file_size, temp);
//     }
//     node->size = size;
//     return;
// }

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

void filldir(struct inode* inum, char** dircontent)
{
    unsigned i;
    unsigned j;
    struct entry* walk;
    i = 0;
    j = 0;
    while(i < DIRTB_NUM)
    {
        walk = inum->dir->tb[i];
        while(walk != NULL)
        {
            dircontent[j] = malloc_string(walk->name);
            j = j+1;
            walk = walk->next;
        }
        i = i+1;
    }
    dircontent[j] = NULL;
    return;
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
    ret = malloc_getattr_ret(inum, inum->mode, inum->size, inum->maj, inum->min);
    unlock(inum);
    return ret;
}

