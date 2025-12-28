#ifndef MBALLOC_H_
#define MBALLOC_H_

#include "common.h" // Assumed to contain the structures above

// Define constants used for allocation logic.
#define PAGE_SIZE 4096
#define ALLOC_GRANULARITY 8 // Allocate in chunks of 8 pages

/**
 * @brief Allocates 'len' blocks starting at 'offset' for the given inode.
 *
 * This function handles block allocation for an inode. It first attempts to
 * satisfy the request from pre-allocated blocks stored in a red-black tree.
 * If that fails or is insufficient, it allocates a new, larger region and
 * adds any leftover space to the pre-allocation tree.
 *
 * @param node The inode for which to allocate blocks.
 * @param offset The logical starting block (page) offset within the file.
 * @param len The number of blocks (pages) to allocate.
 */
void mballoc(struct inode *node, unsigned offset, unsigned len);

#endif // MBALLOC_H_