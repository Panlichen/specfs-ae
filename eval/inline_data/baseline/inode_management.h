/* inode_management.h */
#ifndef INODE_MANAGEMENT_H
#define INODE_MANAGEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "util.h"

#include "hashing.h"
#include "lowlevel_file.h"


struct read_ret* inode_read(struct inode* node, unsigned len, unsigned offset);
unsigned inode_write(struct inode *node, const char* buffer, unsigned len, unsigned offset);
void inode_truncate(struct inode *node, unsigned size);
struct inode* find(struct dirtb* dir, char* name);

#endif /* INODE_MANAGEMENT_H */