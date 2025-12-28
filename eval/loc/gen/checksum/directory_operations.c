/**
 * @brief Inserts a file inode into a directory.
 *
 * This function validates the directory's checksum, adds the new file entry,
 * and then updates the checksum to reflect the change.
 *
 * @param cur The inode of the directory to insert into.
 * @param inum The inode of the file being inserted.
 * @param name The name of the file being inserted.
 */
void insert(struct inode* cur, struct inode* inum, char* name)
{
    // Validate the directory's checksum before accessing it.
    if (!checksum_validate(cur)) {
        fprintf(stderr, "FATAL: Directory checksum validation failed on insert for inode.\n");
        exit(1);
    }

    struct entry* new_entry;
    unsigned hashval;

    hashval = hash_func(name);
    new_entry = malloc_entry();
    new_entry->name = malloc_string(name);
    new_entry->inum = inum;

    // Insert new entry at the beginning of the hash bucket's linked list.
    new_entry->next = cur->dir->tb[hashval];
    cur->dir->tb[hashval] = new_entry;
    cur->size = cur->size + 1;

    // For demonstration, as per original code.
    if (cnt.size < MAX_DIR_SIZE) {
        cnt.files[cnt.size] = inum;
        cnt.size = cnt.size + 1;
    } else {
        fprintf(stderr, "Directory size exceeded\n");
        exit(1);
    }

    // Update the directory's checksum after modification.
    checksum_update(cur);

    return;
}

/**
 * @brief Deletes a file from a directory.
 *
 * This function validates the directory's checksum, finds and removes the
 * specified file entry, and then updates the checksum if a deletion occurred.
 *
 * @param inum The inode of the directory to delete from.
 * @param name The name of the file to delete.
 * @return The inode of the deleted file, or NULL if not found.
 */
struct inode* delete(struct inode* inum, char* name)
{
    // Validate the directory's checksum before accessing it.
    if (!checksum_validate(inum)) {
        fprintf(stderr, "FATAL: Directory checksum validation failed on delete for inode.\n");
        exit(1);
    }

    if (inum->dir == NULL) {
        return NULL;
    }

    unsigned int hashval = hash_func(name);
    // Use a pointer-to-pointer to simplify unlinking from the list.
    struct entry** indirect = &(inum->dir->tb[hashval]);
    struct entry* cur = *indirect;

    while (cur != NULL) {
        // If the names match, the entry is found.
        if (strcmp(cur->name, name) == 0) {
            struct inode* ret = cur->inum;

            // Unlink the current entry from the list.
            *indirect = cur->next;

            free_entry(cur);
            inum->size = inum->size - 1;

            // Update the directory's checksum after modification.
            checksum_update(inum);

            return ret;
        }
        // Move to the next entry in the list.
        indirect = &(cur->next);
        cur = cur->next;
    }

    // Return NULL if the file was not found.
    return NULL;
}