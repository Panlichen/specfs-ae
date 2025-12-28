/**
 * @brief Allocates and encrypts zero-filled blocks for a file.
 */
void file_allocate(struct indextb *tb, unsigned offset, unsigned len) {
    if (len == 0 || tb == NULL || tb->parent_inode == NULL) {
        return;
    }

    unsigned start_page = offset / PG_SIZE;
    unsigned end_page = (offset + len - 1) / PG_SIZE;
    struct inode *parent = tb->parent_inode;
    const unsigned char *key = parent->key;

    for (unsigned page = start_page; page <= end_page; page++) {
        if (tb->index[page] == NULL) {
            // Allocate a new block, zero it out, and encrypt it
            unsigned char *zero_block = (unsigned char*)malloc(PG_SIZE);
            if (zero_block == NULL) {
                // Handle allocation failure
                perror("Failed to allocate memory for zero block");
                return;
            }
            memset(zero_block, 0, PG_SIZE);

            void *encrypted_block = encrypt_block(zero_block, key);
            free(zero_block); // Free the temporary plaintext block

            if (encrypted_block == NULL) {
                // Handle encryption failure
                fprintf(stderr, "Encryption failed for page %u\n", page);
                continue;
            }
            
            tb->index[page] = encrypted_block;
            usleep(1000); // Simulate disk I/O latency
        }
    }
}

/**
 * @brief Reads, decrypts, and returns data from a file.
 */
void file_read(struct indextb *tb, unsigned offset, unsigned len, char *data) {
    if (len == 0 || data == NULL || tb == NULL || tb->parent_inode == NULL) {
        return;
    }

    struct inode *parent = tb->parent_inode;
    const unsigned char *key = parent->key;
    unsigned current_offset = offset;
    unsigned remaining_len = len;
    char *current_data_ptr = data;

    while (remaining_len > 0) {
        unsigned page = current_offset / PG_SIZE;
        unsigned offset_in_page = current_offset % PG_SIZE;
        unsigned bytes_to_copy = PG_SIZE - offset_in_page;
        if (bytes_to_copy > remaining_len) {
            bytes_to_copy = remaining_len;
        }

        if (tb->index[page] == NULL) {
            // Unallocated block reads as zeros
            memset(current_data_ptr, 0, bytes_to_copy);
        } else {
            // Decrypt block and copy data
            void *decrypted_block = decrypt_block(tb->index[page], key);
            if (decrypted_block == NULL) {
                fprintf(stderr, "Decryption failed for page %u\n", page);
                memset(current_data_ptr, 0, bytes_to_copy); // Treat as zeros on failure
            } else {
                memcpy(current_data_ptr, (char *)decrypted_block + offset_in_page, bytes_to_copy);
                free_block(decrypted_block); // Free the decrypted block
            }
        }

        current_data_ptr += bytes_to_copy;
        current_offset += bytes_to_copy;
        remaining_len -= bytes_to_copy;
    }
}

/**
 * @brief Encrypts and writes data to a file.
 */
void file_write(struct indextb *tb, unsigned offset, unsigned len, const char *data) {
    if (len == 0 || data == NULL || tb == NULL || tb->parent_inode == NULL) {
        return;
    }

    struct inode *parent = tb->parent_inode;
    const unsigned char *key = parent->key;
    unsigned current_offset = offset;
    unsigned remaining_len = len;
    const char *current_data_ptr = data;

    while (remaining_len > 0) {
        unsigned page = current_offset / PG_SIZE;
        unsigned offset_in_page = current_offset % PG_SIZE;
        unsigned bytes_to_copy = PG_SIZE - offset_in_page;
        if (bytes_to_copy > remaining_len) {
            bytes_to_copy = remaining_len;
        }

        void *block_plaintext;

        // Get the current plaintext of the block
        if (tb->index[page] != NULL) {
            // Decrypt existing block to modify it
            block_plaintext = decrypt_block(tb->index[page], key);
            if (block_plaintext == NULL) {
                fprintf(stderr, "Decryption failed for page %u, write aborted.\n", page);
                return; 
            }
        } else {
            // Allocate a new block and zero it out if it doesn't exist
            block_plaintext = malloc(PG_SIZE);
            if (block_plaintext == NULL) {
                perror("Failed to allocate memory for new block");
                return;
            }
            memset(block_plaintext, 0, PG_SIZE);
        }

        // Modify the plaintext with the new data
        memcpy((char *)block_plaintext + offset_in_page, current_data_ptr, bytes_to_copy);

        // Encrypt the modified block
        void *encrypted_block = encrypt_block(block_plaintext, key);
        free_block(block_plaintext); // Free the temporary plaintext block

        if (encrypted_block == NULL) {
            fprintf(stderr, "Encryption failed for page %u, write aborted.\n", page);
            return;
        }

        // Free the old encrypted block if it exists
        if (tb->index[page] != NULL) {
            free_block(tb->index[page]);
        }

        // Store the new encrypted block
        tb->index[page] = encrypted_block;

        current_data_ptr += bytes_to_copy;
        current_offset += bytes_to_copy;
        remaining_len -= bytes_to_copy;
    }
}