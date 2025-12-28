unsigned inode_write(struct inode* node, const char* buffer, unsigned len, unsigned offset) {
    // Get the most up-to-date inode from the log.
    struct inode* logged_node = (struct inode*)log_get(node, LOG_TYPE_INODE);
    if (!logged_node) {
        return 0; // Or handle error appropriately
    }

    unsigned clamped_len = len;
    unsigned new_size = max(logged_node->size, offset + len);

    // Clamp length and new size if they exceed the maximum file size.
    if (offset + len > MAX_FILE_SIZE) {
        clamped_len = MAX_FILE_SIZE - offset;
        new_size = MAX_FILE_SIZE;
    }

    // Allocate more space if the write extends the file.
    if (new_size > logged_node->size) {
        file_allocate(logged_node->file, logged_node->size, new_size - logged_node->size);
        clear_file(logged_node, logged_node->size, new_size - logged_node->size);
        logged_node->size = new_size;
    }

    // Write the buffer to the file.
    file_write(logged_node->file, offset, clamped_len, buffer);
    return clamped_len;
}

void inode_truncate(struct inode* node, unsigned size) {
    // Get the most up-to-date inode from the log.
    struct inode* logged_node = (struct inode*)log_get(node, LOG_TYPE_INODE);
    if (!logged_node) {
        return; // Or handle error appropriately
    }
    
    unsigned new_size = (size > MAX_FILE_SIZE) ? MAX_FILE_SIZE : size;

    if (new_size > logged_node->size) {
        // Allocate more space if the new size is larger.
        file_allocate(logged_node->file, logged_node->size, new_size - logged_node->size);
        clear_file(logged_node, logged_node->size, new_size - logged_node->size);
    } else if (new_size < logged_node->size) {
        // Clear the truncated part of the file if the new size is smaller.
        clear_file(logged_node, new_size, logged_node->size - new_size);
    }

    logged_node->size = new_size;
}