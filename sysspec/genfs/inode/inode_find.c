#include "inode.h"

struct inode *inode_find(struct inode *node, char *name)
{
    unsigned int bucket = hash_name(name);
    struct entry *cur = node->dir->tb[bucket];
    
    while (cur != NULL) {
        if (strcmp(cur->name, name) == 0) {
            return (struct inode *)cur->inum;
        }
        cur = cur->next;
    }
    
    return NULL;
}