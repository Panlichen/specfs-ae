#include "inode.h"

struct inode* inode_delete(struct inode* cur, char* name) {
    unsigned int bucket = hash_name(name);
    struct entry *prev = NULL;
    struct entry *curr = cur->dir->tb[bucket];
    
    while (curr != NULL) {
        if (strcmp(curr->name, name) == 0) {
            if (prev == NULL) {
                cur->dir->tb[bucket] = curr->next;
            } else {
                prev->next = curr->next;
            }
            
            struct inode *deleted_inum = (struct inode *)curr->inum;
            free_entry(curr);
            cur->size--;
            return deleted_inum;
        }
        
        prev = curr;
        curr = curr->next;
    }
    
    return NULL;
}