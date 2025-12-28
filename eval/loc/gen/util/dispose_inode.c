#include "util.h"

/**
 * @brief Deallocates all resources associated with a given inode.
 *
 * @param inum A pointer to the `inode` structure to be disposed of. This is the
 *             pre-condition for the function.
 */
void dispose_inode(struct inode* inum) {
    // Per the pre-condition, `inum` is a pointer to an inode. As a safeguard,
    // handle the case where a NULL pointer might be passed.
    if (inum == NULL) {
        return;
    }

    if (inum->mode == FILE_MODE && inum->file != NULL) {
        // The inode represents a file. We need to free its data blocks and index table.
        // Iterate through each entry in the file's index table.
        for (int i = 0; i < INDEXTB_NUM; i++) {
            // Check if the index entry points to an allocated data page.
            if (inum->file->index[i] != NULL) {
                // Free the data page.
                free(inum->file->index[i]);
            }
        }
        // Once all data pages are freed, free the `indextb` structure itself.
        free(inum->file);
    }
    else if (inum->mode == DIR_MODE && inum->dir != NULL) {
        // The inode represents a directory. We need to free its directory table and all entries.
        // Iterate through each bucket in the directory's hash table.
        for (int i = 0; i < DIRTB_NUM; i++) {
            // Get the head of the linked list of entries for the current bucket.
            struct entry* current = inum->dir->tb[i];
            // Traverse the linked list until the end.
            while (current != NULL) {
                struct entry* next = current->next;
                free(current->name);
                free(current);
                current = next;
            }
        }
        // After freeing all entries in all buckets, free the `dirtb` structure.
        free(inum->dir);
    }

    mcs_mutex_destroy(inum->impl);

    free(inum);
}