/* path_handling.h */
#ifndef PATH_HANDLING_H
#define PATH_HANDLING_H

#include "common.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "inode_management.h"

struct inode* locate(struct inode* cur, char* path[]);
char** calculate(char* srcpath[], char* dstpath[]);
int getlen(char* path[]);

#endif /* PATH_HANDLING_H */