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

#ifndef __ATOMFS_H_
#define __ATOMFS_H_


int atomfs_ins(char* path[], char* name, int mode, unsigned maj, unsigned min);
int atomfs_del(char* path[], char* name);
int atomfs_rename(char* srcpath[], char* dstpath[], char* srcname, char* dstname);
struct read_ret* atomfs_read(char** path, unsigned size, unsigned offset);
int atomfs_write(char* path[], const char* buf, unsigned size, unsigned offset);
int atomfs_truncate(char* path[], unsigned offset);
char** atomfs_readdir(char* path[]);
struct inode* atomfs_open(char* path[], unsigned mode);
struct getattr_ret* atomfs_getattr(char* path[]);

#endif
