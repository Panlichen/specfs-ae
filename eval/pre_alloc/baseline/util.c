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

#define _GNU_SOURCE
#include "common.h"
#include "util.h"
#include "log.h"
#include "mcs.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

char* malloc_string(const char* name)
{
    assert(name);
    char* ret;
    unsigned len = strlen(name) + 1;
    ret = malloc( len );
    memset(ret, 0, len);
    strcpy(ret, name);
    return ret;
}

void free_dirs(char* dirname[])
{
    int i = 0;
    for(; dirname[i] != NULL; ++i)
        free(dirname[i]);

    return;
}



void splitDirsFile(const char *path, char* dirname[], char *filename)
{
	char *input;
	char *saveptr;
	unsigned num = 0;
	char *start;

	input = (char *)malloc(strlen(path) + 1);
	memset(input, 0, strlen(path) + 1);
	strcpy(input, path);

	start = strtok_r(input, "/", &saveptr);
	while (start) {
		assert(num < MAX_PATH_LEN -1);
        assert(strlen(start) < MAX_FILE_LEN);
        dirname[num] = malloc_string(start);
		/*log_pmsg("dirname[%d]=%s\n", num, dirname[num]);*/
		++num;
		start = strtok_r(NULL, "/", &saveptr);
	}

    free(input);

	if(num == 0)
		return; 
	num--;
    assert(strlen(dirname[num]) < MAX_FILE_LEN);
	strcpy(filename, dirname[num]);
    free(dirname[num]);
    dirname[num] = NULL;
	return;
}

void splitDirs(const char *path, char* dirname[])
{
	char *input;
	char *saveptr;
	unsigned num = 0;
	char *start;

	input = (char *)malloc(strlen(path) + 1);
	memset(input, 0, strlen(path) + 1);
	strcpy(input, path);

	start = strtok_r(input, "/", &saveptr);
	while (start) {
		assert(num < MAX_PATH_LEN-1);
		assert(strlen(start) < MAX_FILE_LEN);
        dirname[num] = malloc_string(start);
		/*log_pmsg("dirname[%d]=%s\n", num, dirname[num]);*/
		++num;
		start = strtok_r(NULL, "/", &saveptr);
	}
	free(input);
	return;
}

void lock(struct inode* inum)
{

#ifdef LOCK_COUPLING
    mcs_node_t* me = malloc(sizeof(mcs_node_t));
    mcs_mutex_lock(inum->impl, me);
    inum->hd = me;
	pid_t tid = syscall(SYS_gettid);
    inum->mutex = tid;
#endif
    return;
}

void unlock(struct inode* inum)
{
#ifdef LOCK_COUPLING
    mcs_node_t* cur = inum->hd;
    inum->mutex = 0;
    assert(cur);
    mcs_mutex_unlock(inum->impl, cur);
    free(cur);
#endif
	return;
}

struct inode* malloc_inode(int mode, unsigned maj, unsigned min)
{
    struct inode* ret = malloc(sizeof(struct inode));
    memset(ret, 0, sizeof(struct inode));
    ret->mode = mode;
    ret->maj = maj;
    ret->min = min;
    ret->impl = mcs_mutex_create();
    if (mode != DIR_MODE) {
        ret->extents = NULL; // Initialize as an empty list
    } else {
        ret->dir = malloc(sizeof(struct dirtb));
        memset(ret->dir, 0, sizeof(struct dirtb));
    }
    return ret;
}

void dispose_inode(struct inode* inum)
{
    if (inum->mode != DIR_MODE) {
        Extent *current = inum->extents;
        while (current) {
            Extent *next = current->next;
            free(current);
            current = next;
        }
        inum->extents = NULL; // Ensure it's set to NULL after freeing
    } else {
        free(inum->dir);
    }
    mcs_mutex_destroy(inum->impl);
    free(inum);
    return;
}

char** malloc_dir_content(unsigned size)
{
    return malloc( sizeof(char*) * size );
}

struct getattr_ret* malloc_getattr_ret(struct inode* inum, unsigned mode, unsigned size, unsigned maj, unsigned min)
{
    struct getattr_ret* ret = malloc(sizeof(getattr_ret));
    ret->inum = inum;
    if(mode == DIR_MODE)
    {
        ret->maj = 0;
        ret->min = 0;
    }
    else
    {
        ret->maj = maj;
        ret->min = min;
    }
    ret->mode = mode;
    ret->size = size;
    return ret;
}

struct entry* malloc_entry()
{
    return malloc(sizeof(struct entry));
}

char** malloc_path(unsigned len)
{
    char** ret = malloc( len * sizeof(char*) );
    memset(ret, 0, len * sizeof(char*));
    return ret;
}

void free_path(char** path)
{
    unsigned i = 0;
    while(path[i])
    {
        free(path[i]);
        i++;
    }
    free(path);
    return;
}

struct read_ret* malloc_readret()
{
    struct read_ret* ret = malloc( sizeof(struct read_ret) );
    memset(ret, 0, sizeof(struct read_ret));
    return ret;
}

void free_readret(struct read_ret* p)
{
    free(p->buf);
    free(p);
    return;
}

char* malloc_buffer(unsigned len)
{
    return malloc( len * sizeof(char) );
}

unsigned char* malloc_page()
{
    unsigned char* ret;
    ret = malloc(PG_SIZE);
    memset(ret, 0, PG_SIZE);
    return ret;
}

unsigned int min(unsigned int a, unsigned int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

unsigned int max(unsigned int a, unsigned int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}