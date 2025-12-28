#ifndef MBALLOC_H
#define MBALLOC_H

#include "common.h"

#define ALLOC_GRANULARITY 64

unsigned try_prealloc(struct inode *node, unsigned offset, unsigned len);
void mballoc(struct inode *node, unsigned offset, unsigned len);

#endif // MBALLOC_H