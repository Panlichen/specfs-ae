#ifndef DIRECTORY_OPERATIONS_H
#define DIRECTORY_OPERATIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "hashing.h"
#include "util.h"
#include "inode_management.h"

void insert(struct inode* cur, struct inode* inum, char* name);
struct inode* delete(struct inode* inum, char* name);
int check_ins(struct inode* cur, char* name);
int check_del(struct inode* cur, char* name);

#endif