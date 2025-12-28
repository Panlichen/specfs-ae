/**
 * @brief Reads a block of data from a file's index table.
 *
 * It first validates the inode's checksum to ensure data integrity before reading.
 * If a page is not allocated, it is treated as a page of zeros.
 *
 * @param tb Pointer to the index table of the file.
 * @param offset The starting byte offset to read from.
 * @param len The number of bytes to read.
 * @param data A buffer to store the read data.
 */
void file_read(struct indextb *tb, unsigned offset, unsigned len, char *data) {
    assert(tb != NULL && tb->parent_inode != NULL && data != NULL);

    // 1. Validate the checksum before reading any data.
    if (!checksum_validate(tb->parent_inode)) {
        // Halt immediately if data corruption is detected.
        assert("Checksum validation failed before read" && false);
    }

    unsigned current_offset = offset;
    unsigned remaining_len = len;

    while (remaining_len > 0) {
        unsigned page_idx = current_offset / PG_SIZE;
        if (page_idx >= INDEXTB_NUM) {
            // Reading past the maximum file size; treat as zeros.
            memset(data, 0, remaining_len);
            break;
        }

        unsigned offset_in_page = current_offset % PG_SIZE;
        unsigned bytes_to_copy = PG_SIZE - offset_in_page;
        if (bytes_to_copy > remaining_len) {
            bytes_to_copy = remaining_len;
        }

        // If the page doesn't exist, read as zeros. Otherwise, copy the data.
        if (tb->index[page_idx] == NULL) {
            memset(data, 0, bytes_to_copy);
        } else {
            memcpy(data, tb->index[page_idx] + offset_in_page, bytes_to_copy);
        }

        data += bytes_to_copy;
        current_offset += bytes_to_copy;
        remaining_len -= bytes_to_copy;
    }
    // A read operation does not modify the file, so no checksum update is needed.
}

/**
 * @brief Writes a block of data to a file.
 *
 * It validates the checksum before modification, ensures all necessary pages are
 * allocated, writes the data, and finally updates the checksum to reflect the changes.
 *
 * @param tb Pointer to the index table of the file.
 * @param offset The starting byte offset to write to.
 * @param len The number of bytes to write.
 * @param data A buffer containing the data to write.
 */
void file_write(struct indextb *tb, unsigned offset, unsigned len, const char *data) {
    assert(tb != NULL && tb->parent_inode != NULL && data != NULL);

    // 1. Validate the checksum before modifying the file.
    if (!checksum_validate(tb->parent_inode)) {
        assert("Checksum validation failed before write" && false);
    }

    // 2. Ensure all required pages are allocated before writing.
    // file_allocate will manage its own checksum logic if it makes changes.
    if (len > 0) {
        file_allocate(tb, offset, len);
    }

    // 3. Write the data to the pages.
    unsigned current_offset = offset;
    unsigned remaining_len = len;

    while (remaining_len > 0) {
        unsigned page_idx = current_offset / PG_SIZE;
        if (page_idx >= INDEXTB_NUM) {
            // Should not happen if file_allocate works correctly.
            assert("Write attempt beyond maximum file size" && false);
            break;
        }
        
        // The page must exist after the call to file_allocate.
        assert(tb->index[page_idx] != NULL);

        unsigned offset_in_page = current_offset % PG_SIZE;
        unsigned bytes_to_copy = PG_SIZE - offset_in_page;
        if (bytes_to_copy > remaining_len) {
            bytes_to_copy = remaining_len;
        }

        memcpy(tb->index[page_idx] + offset_in_page, data, bytes_to_copy);

        data += bytes_to_copy;
        current_offset += bytes_to_copy;
        remaining_len -= bytes_to_copy;
    }

    // 4. Update the checksum after all modifications are complete.
    if (len > 0) {
        checksum_update(tb->parent_inode);
    }
}

/**
 * @brief Ensures that memory pages for a given range in a file are allocated.
 *
 * It validates the checksum before allocating. If any new pages are allocated,
 * the inode's checksum is updated to reflect the change in metadata.
 *
 * @param tb Pointer to the index table of the file.
 * @param offset The starting byte offset of the range to allocate.
 * @param len The length of the range in bytes.
 */
void file_allocate(struct indextb *tb, unsigned offset, unsigned len) {
    assert(tb != NULL && tb->parent_inode != NULL);

    // 1. Validate the checksum before making changes.
    if (!checksum_validate(tb->parent_inode)) {
        assert("Checksum validation failed before allocation" && false);
    }

    if (len == 0) {
        return;
    }

    unsigned start_page = offset / PG_SIZE;
    unsigned end_page = (offset + len - 1) / PG_SIZE;
    bool modified = false;

    for (unsigned page = start_page; page <= end_page; page++) {
        if (page >= INDEXTB_NUM) {
            break; // Stop if we exceed the maximum number of pages.
        }
        if (tb->index[page] == NULL) {
            tb->index[page] = malloc_page();
            // It's good practice to zero-out newly allocated pages.
            memset(tb->index[page], 0, PG_SIZE);
            modified = true;
        }
    }

    // 2. Update checksum only if new pages were actually allocated.
    if (modified) {
        checksum_update(tb->parent_inode);
    }
}