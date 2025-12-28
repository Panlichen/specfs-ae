#include "file.h"

/**
 * @brief Clears a specified range within a file by writing zeroes.
 *
 * @details This function zeroes out a segment of a file's data. It implements this
 *          by delegating the operation to the `file_write()` function. As per the
 *          [RELY] specification, passing a `NULL` data pointer to `file_write()`
 *          causes it to write zeroes instead of data from a buffer, effectively
 *          clearing the specified region.
 *
 */
void file_clear(struct inode *node, unsigned start, unsigned len)
{
    // Delegate the clearing operation to file_write. Passing NULL as the final
    // argument instructs file_write to fill the specified file range
    // [start, start + len) with zeroes instead of copying from a data buffer.
    file_write(node, start, len, NULL);
}