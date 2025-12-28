#include "file.h"

/**
 * @brief Reads a specified number of bytes from a file represented by an inode.
 * @details This function reads `len` bytes starting from `offset` within the file
 *          associated with the given `inode`. The file's data is assumed to be
 *          stored in a series of 4096-byte blocks (PG_SIZE), which are pointed to
 *          by the `node->file->index` table. The function handles reads that may
 *          span multiple data blocks.
 *
 * @param node A pointer to the inode structure representing the file.
 * @param offset The starting position (in bytes) within the file to begin reading from.
 * @param len The total number of bytes to read.
 * @param data A pointer to the destination buffer where the read data will be stored.
 *
 */
void file_read(struct inode *node, unsigned offset, unsigned len, char *data)
{
    unsigned char *tb_index;
    
    // According to the specification, file data is stored in blocks of PG_SIZE (4096 bytes).
    // Calculate the index of the first block needed for the read operation.
    // `offset >> 12` is equivalent to `offset / 4096`.
    unsigned start_block = offset >> 12;

    // Calculate the index of the last block needed. `offset + len - 1` gives the
    // absolute file offset of the last byte to be read.
    unsigned end_block = (offset + len - 1) >> 12;

    // `cur_offset` tracks the current absolute position in the file as we read.
    unsigned cur_offset = offset;
    
    // `bytes_copied` tracks the total number of bytes written to the `data` buffer.
    unsigned bytes_copied = 0;

    // Iterate through each required block from start to end.
    for (unsigned block = start_block; block <= end_block; block++) {
        // Get the pointer to the current data block from the inode's index table.
        // This relies on the pre-condition that all required blocks are allocated.
        tb_index = node->file->index[block];

        // --- Calculate the portion of this specific block to copy ---

        // Calculate the absolute file offset corresponding to the start of the current block.
        // `block << 12` is equivalent to `block * 4096`.
        unsigned block_start = block << 12;

        // Determine the starting offset *within the current block*.
        // For the first block, this will be `offset % PG_SIZE`.
        // For all subsequent blocks, `cur_offset` will equal `block_start`, so this will be 0.
        unsigned copy_start = (cur_offset > block_start) ? (cur_offset - block_start) : 0;

        // Assume we will copy until the end of the block (offset 4095).
        unsigned copy_end = PG_SIZE - 1;

        // Calculate the absolute file offset of the last byte requested in the entire read operation.
        unsigned total_end = offset + len - 1;

        // If the end of the current block extends past the end of the requested read length,
        // truncate `copy_end` to be the offset of the last byte required *within this block*.
        if (block_start + copy_end > total_end) {
            copy_end = total_end - block_start;
        }
        
        // Calculate the number of bytes to copy from the current block.
        unsigned copy_len = copy_end - copy_start + 1;

        // --- Perform the copy for the current block ---

        // Copy the calculated number of bytes from the source block (`tb_index`)
        // into the destination buffer (`data`).
        // Note: A call to `memcpy(&data[bytes_copied], &tb_index[copy_start], copy_len);`
        // would be more efficient here.
        for (unsigned i = 0; i < copy_len; i++) {
            data[bytes_copied + i] = tb_index[copy_start + i];
        }

        // --- Update state for the next iteration ---

        // Increment the total number of bytes copied to the destination buffer.
        bytes_copied += copy_len;
        
        // Advance the current file offset by the number of bytes we just read.
        cur_offset += copy_len;
    }
}