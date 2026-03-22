#include "file.h"

void file_read(struct inode *node, unsigned offset, unsigned len, char *data) {
    unsigned start_page = offset >> 12;
    unsigned end_page = (offset + len - 1) >> 12;
    
    char *dest = data;
    unsigned bytes_copied = 0;
    
    for (unsigned page_idx = start_page; page_idx <= end_page; page_idx++) {
        unsigned char *page = node->file->index[page_idx];
        
        unsigned page_offset = 0;
        unsigned copy_len = PG_SIZE;
        
        if (page_idx == start_page) {
            page_offset = offset & (PG_SIZE - 1);
            copy_len = PG_SIZE - page_offset;
        }
        
        if (bytes_copied + copy_len > len) {
            copy_len = len - bytes_copied;
        }
        
        for (unsigned i = 0; i < copy_len; i++) {
            dest[bytes_copied + i] = page[page_offset + i];
        }
        
        bytes_copied += copy_len;
    }
}