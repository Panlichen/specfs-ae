#ifndef _LOWLEVEL_FILE_H
#define _LOWLEVEL_FILE_H

#include "common.h"

// File operations
void file_allocate(struct inode *inode, unsigned offset, unsigned len);
void file_read(struct inode *inode, unsigned offset, unsigned len, char *data);
void file_write(struct inode *inode, unsigned offset, unsigned len, const char *data);
void clear_file(struct inode *inode, unsigned start, unsigned len);

// Utility function prototypes
// static Extent* find_extent(Extent *head, unsigned page);
// static int is_page_allocated(Extent *head, unsigned page);

#endif /* _LOWLEVEL_FILE_H */