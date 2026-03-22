#include "inode.h"

struct read_ret* inode_read(struct inode* node, unsigned len, unsigned offset) {
    struct read_ret* ret = malloc_readret();
    
    if (offset >= node->size || len == 0) {
        // Case 1: Empty read range
        ret->num = 0;
        ret->buf = NULL;
        return ret;
    }
    
    // Case 2: Read data
    unsigned remaining = node->size - offset;
    unsigned to_read = (len < remaining) ? len : remaining;
    
    char* buffer = malloc_buffer(to_read);
    file_read(node, offset, to_read, buffer);
    
    ret->num = to_read;
    ret->buf = buffer;
    return ret;
}