#include "inode.h"

/**
 * @brief Deletes a directory entry by name from a parent directory inode.
 *
 * This function searches for a directory entry with the given `name` within the
 * directory represented by the `cur` inode. If found, the entry is removed from
 * the directory's hash table, its memory is freed, the parent directory's size
 * is decremented, and the inode pointer associated with the entry is returned.
 *
 * @param cur A pointer to the parent directory inode.
 * @param name The name of the directory entry to delete.
 *
 * @return On success, a pointer to the inode of the deleted entry.
 * @return On failure or if the entry is not found, NULL.
 */
struct inode* inode_delete(struct inode* cur, char* name)
{
    // Validate pre-conditions: ensure the parent inode, its directory table,
    // and the name are not NULL.
    if (cur == NULL || cur->dir == NULL || name == NULL) {
        return NULL; // Fulfills Case 2 of the post-condition for invalid input.
    }

    // Per the directory well-formedness invariant, calculate the hash bucket
    // where the entry should reside.
    unsigned int bucket_index = hash_name(name);

    // Get a pointer to the head of the linked list for the calculated bucket.
    // Using a pointer-to-pointer allows modification of the head itself if the
    // first element is the one to be deleted.
    struct entry **head = &(cur->dir->tb[bucket_index]);
    struct entry *prev = NULL;
    struct entry *current = *head;

    // Traverse the linked list within the hash bucket.
    while (current != NULL) {
        // Compare the current entry's name with the target name.
        if (strcmp(current->name, name) == 0) {
            // Entry found. Begin Case 1 of the post-condition.

            // Store the inode pointer from the entry to be returned later.
            struct inode *found_inum = (struct inode *)current->inum;

            // Unlink the entry from the list.
            if (prev == NULL) {
                // The entry to be deleted is the first one in the bucket.
                // Update the head of the list to point to the next entry.
                *head = current->next;
            } else {
                // The entry is in the middle or at the end of the list.
                // Bypass the current entry by linking the previous one to the next one.
                prev->next = current->next;
            }

            // Free the memory occupied by the directory entry structure.
            free_entry(current);

            // Decrement the size of the parent directory.
            cur->size -= 1;

            // Return the inode pointer of the deleted entry, completing Case 1.
            return found_inum;
        }

        // Move to the next entry in the list for the next iteration.
        prev = current;
        current = current->next;
    }

    // If the loop completes, the entry was not found in the bucket.
    // Fulfill Case 2 of the post-condition.
    return NULL;
}