/* inode_management.h */
#ifndef INODE_MANAGEMENT_H
#define INODE_MANAGEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "util.h"
#include "hashing.h"

struct inode* find(struct dirtb* dir, char* name);
struct read_ret* inode_read(struct inode* node, unsigned len, unsigned offset);
unsigned inode_write(struct inode* node, const char* buffer, unsigned len, unsigned offset);
void inode_truncate(struct inode* node, unsigned size);

#endif /* INODE_MANAGEMENT_H */