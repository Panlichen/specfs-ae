#include "inode.h"

/**
 * @brief Reads a specified number of bytes from an inode at a given offset.
 * @details This function implements the `inode_read` operation. It reads data
 *          from an inode, handling cases where the read range is empty or
 *          extends beyond the end of the file. It dynamically allocates a
 *          return structure and a buffer for the data.
 *
 *
 * @param node Pointer to a valid inode from which to read. Per the pre-condition,
 *             its `size` and `file` fields must be initialized.
 * @param len The maximum number of bytes to read.
 * @param offset The starting position within the file to begin reading.
 *
 * @return A pointer to a dynamically allocated `read_ret` structure.
 *         - If data is read, `ret->buf` points to the data and `ret->num` is
 *           the number of bytes read, corresponding to `min(len, node->size - offset)`.
 *         - If the read range is empty (`offset >= node->size` or `len == 0`),
 *           `ret->buf` is `NULL` and `ret->num` is 0.
 *         - The function may return NULL if the initial `read_ret` allocation fails.
 *
 */
struct read_ret* inode_read(struct inode* node, unsigned len, unsigned offset) {
    // [RELY] Allocate the `read_ret` structure that will be returned.
    struct read_ret* ret = malloc_readret();
    
    // A robust check in case the allocation of the return structure fails.
    if (!ret) {
        return NULL;
    }
    
    // [SPECIFICATION] Post-Condition, Case 1: The read range is empty.
    // This occurs if the offset is at or beyond the end of the file, or if the
    // requested length is zero.
    if (offset >= node->size || len == 0) {
        // Set the number of bytes read to 0 and the buffer to NULL as per the spec.
        ret->num = 0;
        ret->buf = NULL;
        return ret;
    }
    
    // [SPECIFICATION] Post-Condition, Case 2: Data is to be read.
    // First, determine the actual number of bytes to read. Start with the requested length.
    unsigned bytes_to_read = len;
    
    // Calculate the number of bytes available from the given offset to the end of the file.
    unsigned available = node->size - offset;
    
    // The number of bytes to read cannot exceed what's available.
    // This logic implements the `min(len, node->size - offset)` requirement.
    if (bytes_to_read > available) {
        bytes_to_read = available;
    }
    
    // [RELY] Allocate a buffer of the precise size needed to hold the data.
    ret->buf = malloc_buffer(bytes_to_read);
    
    // Handle the case where buffer allocation fails.
    if (!ret->buf) {
        // If buffer allocation fails, we cannot read data. We set `num` to 0,
        // indicating no bytes were read, and return the `ret` struct.
        // The caller is still responsible for freeing `ret`.
        ret->num = 0;
        return ret;
    }
    
    // Set the `num` field to the actual number of bytes that will be read.
    ret->num = bytes_to_read;
    
    // [RELY] Use the provided `file_read` function to perform the actual I/O,
    // populating the newly allocated buffer with data from the file.
    file_read(node, offset, bytes_to_read, ret->buf);
    
    // Return the structure containing the buffer and the number of bytes read.
    return ret;
}