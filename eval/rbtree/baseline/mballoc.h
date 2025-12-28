#ifndef MBALLOC_H
#define MBALLOC_H

#include "common.h"

#define ALLOC_GRANULARITY 64

static void create_extent(struct inode *node, unsigned start, unsigned length, unsigned char *data);
void mballoc(struct inode *node, unsigned offset, unsigned len);

#endif // MBALLOC_H