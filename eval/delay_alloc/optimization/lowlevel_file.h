#ifndef _LOWLEVEL_FILE_H
#define _LOWLEVEL_FILE_H

#include "common.h"
#include "util.h"

// File operations
void file_allocate(struct indextb *tb, unsigned offset, unsigned len);
void file_read(struct indextb *tb, unsigned offset, unsigned len, char *data);
void file_write(struct indextb *tb, unsigned offset, unsigned len, const char *data);
void clear_file(struct inode *node, unsigned start, unsigned len);

#endif /* _LOWLEVEL_FILE_H */