#include "file.h"

void file_allocate(struct inode *node, unsigned offset, unsigned len)
{
    // Calculate starting page index
    unsigned start_page = offset / PG_SIZE;
    
    // Calculate ending page index (handle the last byte position)
    unsigned end_page = (offset + len - 1) / PG_SIZE;
    
    // Allocate pages for each index in range
    for (unsigned i = start_page; i <= end_page; i++) {
        // Check if page is not already allocated
        if (node->file->index[i] == NULL) {
            // Allocate a new page
            node->file->index[i] = malloc_page();
        }
    }
}