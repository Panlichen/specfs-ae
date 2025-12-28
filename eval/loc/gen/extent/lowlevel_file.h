#ifndef LOWLEVEL_FILE_H
#define LOWLEVEL_FILE_H

#include "common.h"
#include <stddef.h> // For size_t

/**
 * @brief Allocates 'num_pages' contiguous, zero-initialized 4KB pages.
 * This is a crucial utility for creating efficient extents.
 */
unsigned char* malloc_pages(unsigned num_pages);


/**
 * @brief Ensures storage is allocated for a file region.
 * If pages are not already allocated for the range [offset, offset + len),
 * a new extent is created and appended to the inode's extent list.
 */
void file_allocate(struct inode *node, unsigned offset, unsigned len);

/**
 * @brief Reads data from a file using its extent map.
 * Reads 'len' bytes from 'offset' into the 'data' buffer. Regions within the
 * file that have not been allocated (holes) are read as zeros. This function
 * merges reads within a single extent for better performance.
 */
void file_read(struct inode *node, unsigned offset, unsigned len, char *data);

/**
 * @brief Writes data to a file using its extent map.
 * Writes 'len' bytes from 'data' to 'offset'. This will allocate new extents
 * if necessary and updates the file size if the write extends past the current EOF.
 * This function merges writes within a single extent.
 */
void file_write(struct inode *node, unsigned offset, unsigned len, const char *data);

/**
 * @brief Writes zeros to a specified file region.
 * A convenience function that uses file_write with a NULL data source.
 */
void clear_file(struct inode *node, unsigned start, unsigned len);

/**
 * @brief Frees all memory associated with an inode's extents.
 * This function is critical for preventing memory leaks upon file deletion.
 */
void free_all_extents(struct inode *node);


#endif // LOWLEVEL_FILE_H