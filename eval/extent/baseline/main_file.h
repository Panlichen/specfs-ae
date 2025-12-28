#ifndef MAIN_FILE_H
#define MAIN_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "util.h"


#include "inode_management.h"
#include "path_handling.h"

struct read_ret* atomfs_read(char** path, unsigned size, unsigned offset);
int atomfs_write(char* path[], const char* buf, unsigned size, unsigned offset);
int atomfs_truncate(char* path[], unsigned offset);
struct inode* atomfs_open(char* path[], unsigned mode);


#endif