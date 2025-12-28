#ifndef MBALLOC_H
#define MBALLOC_H

#include "common.h"

/**
 * @brief Allocates 'len' blocks starting at 'offset' for the given inode.
 *
 * This function handles block allocation for an inode. It first attempts to
 * satisfy the request from pre-allocated blocks. If the pre-allocated space
 * is insufficient, it falls back to allocating a new contiguous memory region.
 * The size of this new region is rounded up to the nearest multiple of
 * ALLOC_GRANULARITY. Any extra space from this new allocation is saved as a
 * new pre-allocated block for future use.
 *
 * @param node The inode for which to allocate blocks.
 * @param offset The logical starting block (page) offset within the file.
 * @param len The number of blocks (pages) to allocate.
 */
void mballoc(struct inode *node, unsigned offset, unsigned len);

#endif /* MBALLOC_H */