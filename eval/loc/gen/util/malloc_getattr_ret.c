#include "util.h"

/**
 * @brief Allocates and initializes a getattr_ret structure.
 *
 *
 * @param inum  A valid pointer to an `inode` structure.
 * @param mode  The mode of the inode (e.g., FILE_MODE, DIR_MODE).
 * @param size  The size of the inode in bytes.
 * @param maj   The major device number (relevant for device files).
 * @param min   The minor device number (relevant for device files).
 *
 * @return A pointer to the newly allocated `getattr_ret` structure on success.
 * @return NULL if memory allocation fails.
 */
struct getattr_ret* malloc_getattr_ret(struct inode* inum, unsigned mode, unsigned size, unsigned maj, unsigned min)
{
    struct getattr_ret* ret = (struct getattr_ret*)malloc(sizeof(struct getattr_ret));
    
    if (ret == NULL) {
        return NULL;
    }
    
    ret->inum = inum;
    ret->mode = mode;
    ret->size = size;
    
    if (mode == DIR_MODE) {
        ret->maj = 0;
        ret->min = 0;
    } else {
        ret->maj = maj;
        ret->min = min;
    }
    
    return ret;
}