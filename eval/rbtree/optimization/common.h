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

#ifndef __COMMON_H_
#define __COMMON_H_

#define FUSE_USE_VERSION 26
#define _XOPEN_SOURCE 500
#include <string.h>
#include <stdio.h>
// #include "rbtree.h"

#define FS_DATA ((struct fs_state *) fuse_get_context()->private_data)

//#define DEBUG
#define LOCK_COUPLING

#define FILE_MODE 1
#define DIR_MODE 2
#define CHR_MODE 3
#define BLK_MODE 4
#define SOCK_MODE 5
#define FIFO_MODE 6

#define PG_SIZE 4096
#define INDEXTB_NUM 8192
#define DIRTB_NUM 512
#define MAX_FILE_LEN 256
#define MAX_DIR_SIZE 10000000
#define MAX_PATH_LEN 100
#define MAX_FILE_SIZE ((unsigned)(INDEXTB_NUM*PG_SIZE))
#define PAGE_SIZE (4096)

#define EXTENT_DEGREE 4  // 与ext4的extent结点容量一致

// Prealloc structure
typedef struct Prealloc {
    unsigned pa_lstart;
    unsigned char *pa_data;
    unsigned pa_len;
    unsigned pa_free;
} Prealloc;

// Red-black tree node structure
struct rb_node {
    struct rb_node *parent;
    struct rb_node *left;
    struct rb_node *right;
    unsigned color; // Using unsigned for simplicity (0: black, 1: red)
    Prealloc *prealloc; // Pointer to the Prealloc structure
};

typedef struct Extent {
    unsigned start_page;   // Logical starting page in file
    unsigned length;       // Number of contiguous pages
    unsigned char *data;   // Contiguous memory for pages
    struct Extent *next;   // Linked list of extents
} Extent;


typedef unsigned long long u64;
typedef struct entry{
    char* name;
    void* inum;
    struct entry* next;
}entry;

typedef struct indextb{
    unsigned char *index[INDEXTB_NUM];
}indextb;

typedef struct dirtb{
    struct entry* tb[DIRTB_NUM];
}dirtb;

// Inode structure (partial)
typedef struct inode {
    int mutex;
    struct mcs_mutex *impl;
    struct mcs_node *hd;
    unsigned maj;
    unsigned min;
    unsigned int mode;
    unsigned int size;
    struct dirtb* dir;
    Extent *extents;       // Extent list head
    struct rb_node *prealloc_tree;   // Red-black tree for prealloc extents
} inode;

typedef struct getattr_ret{
    struct inode* inum;
    unsigned mode;
    unsigned size;
    unsigned maj;
    unsigned min;
} getattr_ret;

typedef struct read_ret{
    char* buf;
    unsigned num;
} read_ret;

extern struct inode* root_inum;

struct fs_state {
	FILE *logfile;
};

#endif
