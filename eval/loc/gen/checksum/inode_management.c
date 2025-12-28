/**
 * @brief Reads data from an inode at a given offset.
 *
 * It first validates the inode's metadata checksum. If the checksum is valid,
 * it proceeds to read the specified length of data from the file associated
 * with the inode.
 *
 * @param node The inode to read from.
 * @param len The maximum number of bytes to read.
 * @param offset The starting offset within the file to begin reading.
 * @return A pointer to a struct read_ret containing the buffer with the
 * data read and the actual number of bytes read. Returns an
 * empty result if the checksum is invalid or the offset is out of bounds.
 */
struct read_ret* inode_read(struct inode* node, unsigned len, unsigned offset) {
    // 1. Validate the checksum before accessing inode metadata.
    if (!checksum_validate(node)) {
        // Inode metadata is corrupt, return an empty read.
        struct read_ret* ret_fail = malloc_readret();
        ret_fail->num = 0;
        ret_fail->buf = NULL;
        return ret_fail;
    }

    struct read_ret* ret = malloc_readret();
    if (offset >= node->size) {
        ret->num = 0;
        ret->buf = NULL;
        return ret;
    }

    unsigned actual_len = min(len, node->size - offset);
    ret->buf = malloc_buffer(actual_len);
    file_read(node->file, offset, actual_len, ret->buf);
    ret->num = actual_len;

    // No inode metadata was modified, so no checksum update is needed.
    return ret;
}

/**
 * @brief Writes data to an inode at a given offset.
 *
 * It validates the inode's checksum before proceeding. If the write operation
 * extends the file, the inode's size is updated. Finally, it updates the
 * metadata checksum to reflect any changes.
 *
 * @param node The inode to write to.
 * @param buffer A pointer to the data to be written.
 * @param len The number of bytes to write.
 * @param offset The starting offset within the file to begin writing.
 * @return The number of bytes actually written. Returns 0 on checksum failure.
 */
unsigned inode_write(struct inode* node, const char* buffer, unsigned len, unsigned offset) {
    // 1. Validate the checksum before modifying inode metadata.
    if (!checksum_validate(node)) {
        // Inode metadata is corrupt, abort the write.
        return 0;
    }

    unsigned clamped_len = len;
    unsigned new_size = max(node->size, offset + len);

    if (offset + len > MAX_FILE_SIZE) {
        clamped_len = MAX_FILE_SIZE - offset;
        new_size = MAX_FILE_SIZE;
    }

    if (new_size > node->size) {
        file_allocate(node->file, node->size, new_size - node->size);
        clear_file(node, node->size, new_size - node->size);
        // Metadata (size) is being changed.
        node->size = new_size;
    }

    file_write(node->file, offset, clamped_len, buffer);

    // 2. Update the checksum because inode metadata (size) may have changed.
    checksum_update(node);

    return clamped_len;
}


/**
 * @brief Truncates or extends a file to a specified size.
 *
 * It first validates the inode's metadata checksum. It then adjusts the
 * file's size, allocating or clearing space as needed. After the size
 * modification, it updates the inode's checksum to ensure consistency.
 *
 * @param node The inode of the file to truncate.
 * @param size The new target size for the file.
 */
void inode_truncate(struct inode* node, unsigned size) {
    // 1. Validate the checksum before modifying inode metadata.
    if (!checksum_validate(node)) {
        // Inode metadata is corrupt, abort the truncate operation.
        return;
    }

    if (size > node->size) {
        file_allocate(node->file, node->size, size - node->size);
        clear_file(node, node->size, size - node->size);
    } else if (size < node->size) {
        clear_file(node, size, node->size - size);
        // NOTE: A real implementation would also deallocate file blocks here.
    }

    // Metadata (size) is being changed.
    node->size = size;

    // 2. Update the checksum because inode metadata (size) has changed.
    checksum_update(node);
}