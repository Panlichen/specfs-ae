#include "file.h"

void file_write(struct inode *node, unsigned offset, unsigned len, const char *data) {
    unsigned current_offset = offset;
    unsigned remaining = len;
    
    while (remaining > 0) {
        unsigned page_idx = current_offset / PG_SIZE;
        unsigned page_offset = current_offset % PG_SIZE;
        
        unsigned bytes_in_page = PG_SIZE - page_offset;
        if (bytes_in_page > remaining) {
            bytes_in_page = remaining;
        }
        
        if (data == NULL) {
            if (node->file->index[page_idx] == NULL) {
                node->file->index[page_idx] = (unsigned char*)malloc(PG_SIZE);
            }
            memset(node->file->index[page_idx] + page_offset, 0, bytes_in_page);
        } else {
            memcpy(node->file->index[page_idx] + page_offset, data + (len - remaining), bytes_in_page);
        }
        
        current_offset += bytes_in_page;
        remaining -= bytes_in_page;
    }
}