#include "inode.h"

unsigned inode_write(struct inode* node, const char* buffer, unsigned len, unsigned offset)
{
    // Calculate new size after write
    unsigned new_size = offset + len;
    
    // Truncate to maximum file size if needed
    if (new_size > MAX_FILE_SIZE) {
        new_size = MAX_FILE_SIZE;
        len = MAX_FILE_SIZE - offset;
    }
    
    // Extend file if new size exceeds current size
    if (new_size > node->size) {
        file_allocate(node, node->size, new_size - node->size);
        file_clear(node, node->size, new_size - node->size);
        node->size = new_size;
    }
    
    // Write data to file
    file_write(node, offset, len, buffer);
    
    return len;
}