#ifndef DELALLOC_H
#define DELALLOC_H

#include "common.h"
#include "mcs.h"

/**
 * @brief Releases all buffered blocks associated with a specific index table.
 *
 * This is typically called when the file (represented by the index table) is closed
 * or deleted, to remove its pending writes from the buffer. This operation is
 * thread-safe.
 *
 * @param tb Pointer to the index table whose buffered blocks should be released.
 */
void brels(struct indextb *tb);

/**
 * @brief Reads a page associated with an index table.
 *
 * This function is thread-safe. It searches the delayed allocation buffer first.
 * If the page is not found in the buffer, it reads directly from the index table.
 *
 * @param tb Pointer to the index table.
 * @param page The page number to read.
 * @return A pointer to the page data, either in the buffer or from the index table.
 */
unsigned char *bread(struct indextb *tb, unsigned int page);

/**
 * @brief Writes data to a page associated with an index table.
 *
 * The write is buffered in a thread-safe manner. It first searches for the page
 * in the buffer. If found, the data is written to the existing buffer block. If
 * not, a new block is allocated in the buffer. If the buffer is full, it is flushed
 * to disk before the new block is allocated.
 *
 * @param tb Pointer to the index table.
 * @param page The page number to write to.
 * @param offset The offset within the page to start writing.
 * @param buf A pointer to the data to write. If NULL, the target area is zero-filled.
 * @param size The number of bytes to write.
 */
void bwrite(struct indextb *tb, unsigned int page, unsigned int offset, unsigned char *buf, unsigned int size);

#endif // DELALLOC_H