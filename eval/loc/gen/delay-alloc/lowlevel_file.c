/**
 * @brief Prepares a file region for future writing (no-op).
 *
 * In a delayed allocation filesystem, physical blocks are not reserved
 * until data is actually written. Therefore, this function does nothing.
 *
 * PRECONDITION: You are given an indextb, start offset, length to allocate.
 * POSTCONDITION: Do nothing.
 *
 * @param tb Pointer to the index table for the file.
 * @param offset The starting file offset to allocate.
 * @param len The number of bytes to allocate.
 */
void file_allocate(struct indextb *tb, unsigned offset, unsigned len) {
    // This is a no-op due to the delayed allocation strategy.
    // Allocation is deferred until the actual write operation.
    (void)tb;
    (void)offset;
    (void)len;
}

/**
 * @brief Reads data from a file.
 *
 * Reads a block of data of a given length from a specified offset. If it
 * attempts to read from a file block that has not yet been allocated (a "hole"),
 * it correctly returns zeros for that portion of the read.
 *
 * PRECONDITION: You are given an indextb, start offset, length to read and a
 * buffer to store the data.
 * POSTCONDITION: The data is read from the file. Unallocated pages are read as zeros.
 *
 * @param tb Pointer to the index table for the file.
 * @param offset The starting file offset to read from.
 * @param len The number of bytes to read.
 * @param data A pointer to the buffer where the read data will be stored.
 */
void file_read(struct indextb *tb, unsigned offset, unsigned len, char *data) {
    if (len == 0) {
        return;
    }

    unsigned int current_offset = offset;
    unsigned int remaining_len = len;

    while (remaining_len > 0) {
        unsigned int page_num = current_offset / PAGE_SIZE;
        unsigned int offset_in_page = current_offset % PAGE_SIZE;
        
        // Determine how many bytes to process in the current page
        unsigned int bytes_to_process = PAGE_SIZE - offset_in_page;
        if (bytes_to_process > remaining_len) {
            bytes_to_process = remaining_len;
        }

        // bread returns the page buffer or NULL if not allocated
        unsigned char *page_data = bread(tb, page_num);

        if (page_data == NULL) {
            // Page is not allocated; fill the corresponding part of the user buffer with zeros.
            memset(data, 0, bytes_to_process);
        } else {
            // Page is allocated; copy the data.
            memcpy(data, page_data + offset_in_page, bytes_to_process);
        }

        // Advance pointers and counters for the next iteration
        data += bytes_to_process;
        current_offset += bytes_to_process;
        remaining_len -= bytes_to_process;
    }
}

/**
 * @brief Writes data to a file.
 *
 * Writes a block of data to a specified offset. The underlying `bwrite`
 * function is responsible for allocating a new page if one does not already
 * exist for the target address.
 *
 * PRECONDITION: You are given an indextb, start offset, length to write and
 * a buffer with the data.
 * POSTCONDITION: The data is written to the file, and pages are allocated as needed.
 *
 * @param tb Pointer to the index table for the file.
 * @param offset The starting file offset to write to.
 * @param len The number of bytes to write.
 * @param data A pointer to the buffer containing the data to write.
 * If NULL, the corresponding file region will be zero-filled.
 */
void file_write(struct indextb *tb, unsigned offset, unsigned len, const char *data) {
    if (len == 0) {
        return;
    }

    unsigned int current_offset = offset;
    unsigned int remaining_len = len;

    while (remaining_len > 0) {
        unsigned int page_num = current_offset / PAGE_SIZE;
        unsigned int offset_in_page = current_offset % PAGE_SIZE;

        // Determine how many bytes to process in the current page
        unsigned int bytes_to_process = PAGE_SIZE - offset_in_page;
        if (bytes_to_process > remaining_len) {
            bytes_to_process = remaining_len;
        }

        // The bwrite utility handles the write and allocation logic.
        // A type cast is needed because bwrite expects `unsigned char *`.
        bwrite(tb, page_num, offset_in_page, (unsigned char *)data, bytes_to_process);

        // Advance pointers and counters for the next iteration
        if (data != NULL) {
            data += bytes_to_process;
        }
        current_offset += bytes_to_process;
        remaining_len -= bytes_to_process;
    }
}