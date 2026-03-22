#include "inode.h"

int inode_insert(struct inode* cur, struct inode* inum, char* name) {
    // Compute the hash index for the entry
    unsigned int hash_index = hash_name(name) % DIRTB_NUM;
    
    // Allocate a new entry
    struct entry* new_entry = malloc_entry();
    if (new_entry == NULL) {
        return 1;  // Memory allocation failure for entry
    }
    
    // Allocate and copy the name string
    new_entry->name = malloc_string(name);
    if (new_entry->name == NULL) {
        free(new_entry);  // Free the entry if string allocation fails
        return 1;         // Memory allocation failure for name
    }
    
    // Set the entry's inode pointer and next pointer
    new_entry->inum = (void*)inum;
    new_entry->next = cur->dir->tb[hash_index];
    
    // Insert at the head of the bucket list
    cur->dir->tb[hash_index] = new_entry;
    
    // Increase the directory size
    cur->size++;
    
    return 0;  // Success
}