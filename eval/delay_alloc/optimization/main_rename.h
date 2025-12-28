#ifndef MAIN_RENAME_H
#define MAIN_RENAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
// #include "hashing.h"
#include "util.h"

#include "inode_management.h"
#include "path_handling.h"
#include "directory_operations.h"


int atomfs_rename(char* srcpath[], char* dstpath[], char* srcname, char* dstname);


#endif