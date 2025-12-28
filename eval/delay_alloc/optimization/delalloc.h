#ifndef DELALLOC_H
#define DELALLOC_H

#include "common.h"

void brels(struct indextb *tb);
unsigned char *bread(struct indextb *tb, unsigned int page);
void bwrite(struct indextb *tb, unsigned int page, unsigned int offset, unsigned char *buf, unsigned int size);

#endif