#ifndef LOWLEVEL_FILE_H
#define LOWLEVEL_FILE_H

#include "common.h"


/**
 * @brief Ensures that data pages exist for a given range of a file.
 * This function only allocates pages for the portion of the range that
 * falls outside the inline_data buffer.
 *
 * @param node The inode for the file.
 * @param offset The starting offset of the range.
 * @param len The length of the range.
 */
void file_allocate(struct inode *node, unsigned offset, unsigned len);

/**
 * @brief Reads data from a file, handling both inline and paged data.
 *
 * @param node The inode for the file.
 * @param offset The starting offset to read from.
 * @param len The number of bytes to read.
 * @param data The buffer to store the read data into.
 */
void file_read(struct inode *node, unsigned offset, unsigned len, char *data);

/**
 * @brief Writes data to a file, handling both inline and paged data.
 *
 * @param node The inode for the file.
 * @param offset The starting offset to write to.
 * @param len The number of bytes to write.
 * @param data The buffer containing the data to write.
 */
void file_write(struct inode *node, unsigned offset, unsigned len, const char *data);

/**
 * @brief A convenience function to write zeros to a portion of a file.
 *
 * @param node The inode for the file.
 * @param start The starting offset to clear.
 * @param len The number of bytes to clear.
 */
void clear_file(struct inode *node, unsigned start, unsigned len);

/**
 * @brief Allocates a new, zeroed memory page. (Assumed to be implemented elsewhere)
 * @return A pointer to the allocated page, or NULL on failure.
 */
void* malloc_page();

#endif // LOWLEVEL_FILE_H