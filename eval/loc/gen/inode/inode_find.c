#include "inode.h"

/**
 * @brief Finds an inode corresponding to a name within a directory inode.
 *
 * This function searches for a directory entry named 'name' within the directory
 * represented by 'node'. It leverages the directory's hash table structure,
 * as described in the "Directory well-formedness" invariant, to efficiently
 * locate the entry.
 *
 * @param node A pointer to the directory inode to search in.
 *             Based on the Pre-Condition, this must be a valid directory
 *             inode where `node->dir` is not NULL.
 * @param name The name of the directory entry to find.
 *
 * @return A pointer to the found inode if an entry with the given name exists
 *         (fulfilling Post-Condition Case 1).
 * @return NULL if an entry with the given name is not found (fulfilling
 *         Post-Condition Case 2).
 */
struct inode *inode_find(struct inode *node, char *name)
{
    // Per the "Directory well-formedness" invariant, the entry for 'name'
    // must reside in the bucket calculated by its hash value.
    unsigned int bucket = hash_name(name);

    // Get the head of the linked list for the calculated bucket.
    entry *cur = node->dir->tb[bucket];
    
    // Traverse the linked list (collision chain) for this bucket.
    while (cur != NULL) {
        // Compare the current entry's name with the target name.
        if (strcmp(cur->name, name) == 0) {
            // A match is found. Return the associated inode pointer.
            // This satisfies "Post-Condition: Case 1".
            return (struct inode *)cur->inum;
        }
        // Move to the next entry in the chain.
        cur = cur->next;
    }
    
    // If the loop completes, the name was not found in the directory.
    // This satisfies "Post-Condition: Case 2".
    return NULL;
}