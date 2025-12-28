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

#ifndef __UTIL_H_
#define __UTIL_H_

#include "common.h"

void lock(struct inode* inum);
void unlock(struct inode* inum);
struct inode* malloc_inode(int mode, unsigned maj, unsigned min);
void dispose_inode(struct inode* inum);
void splitDirsFile(const char *path, char* dirname[], char *filename);
void splitDirs(const char *path, char* dirname[]);
char* malloc_string(const char* name);
char** malloc_dir_content(unsigned size);
struct getattr_ret* malloc_getattr_ret(struct inode* inum, unsigned mode, unsigned size, unsigned maj, unsigned min);
struct entry* malloc_entry();
char** malloc_path(unsigned len);
void free_path(char** path);
struct read_ret* malloc_readret();
void free_readret(struct read_ret* p);
char* malloc_buffer(unsigned len);
unsigned char* malloc_contigous_pages(unsigned num);
void free_dirs(char* dirname[]);
unsigned char* malloc_page(void);
unsigned int min(unsigned int a, unsigned int b);
unsigned int max(unsigned int a, unsigned int b);

#endif
