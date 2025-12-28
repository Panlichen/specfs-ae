#include "inode.h"

/**
 * @brief Inserts a new directory entry into a directory inode.
 *
 * This function creates a new directory entry with the given name and inode
 * and inserts it into the directory table of the `cur` inode. The entry is
 * placed at the head of the appropriate hash bucket's linked list, determined
 * by hashing the entry's name.
 *
 * @param cur A pointer to the directory inode where the new entry will be inserted.
 * @param inum A pointer to the inode that the new entry will refer to.
 * @param name The name for the new directory entry.
 *
 * @return Returns 0 on successful insertion (Post-Condition Case 1).
 * @return Returns 1 on failure due to memory allocation errors (Post-Condition Case 2).
 *         On failure, any partially allocated memory is freed.
 */
int inode_insert(struct inode* cur, struct inode* inum, char* name) {
    // Calculate the hash bucket index 'n' based on the entry's name. This determines
    // which linked list in the directory table the new entry will be added to.
    unsigned int n = hash_name(name) % DIRTB_NUM;

    // Allocate a new entry struct.
    struct entry* new_entry = malloc_entry();
    // If allocation fails, return 1 as specified for memory errors (Case 2).
    if (!new_entry) {
        return 1;
    }

    // Allocate a separate copy of the name string for the entry to own.
    char* name_copy = malloc_string(name);
    // If the name copy allocation fails, we must free the previously allocated
    // entry to prevent a memory leak before returning the error code (Case 2).
    if (!name_copy) {
        free(new_entry);
        return 1;
    }

    // Populate the new entry with the copied name and the target inode.
    new_entry->name = name_copy;
    new_entry->inum = inum;

    // Insert the new entry at the head of the linked list for the calculated bucket.
    // The new entry's `next` pointer will point to the previous head of the list.
    new_entry->next = cur->dir->tb[n];
    // The head of the list in the directory table is updated to point to the new entry.
    cur->dir->tb[n] = new_entry;

    // As required by the specification (Case 1), increment the directory's size to
    // reflect the new entry.
    cur->size++;

    // Return 0 to indicate the operation was successful (Case 1).
    return 0;
}