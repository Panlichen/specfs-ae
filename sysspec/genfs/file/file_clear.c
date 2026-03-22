#include "file.h"

void file_clear(struct inode *node, unsigned start, unsigned len)
{
    // Write NULL data to clear the specified range
    file_write(node, start, len, NULL);
}