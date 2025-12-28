#include "util.h"

/**
 * @brief Allocates and initializes a new inode structure.
 *
 * @param mode The mode of the inode, which determines if it is a directory (DIR_MODE)
 *             or a regular file.
 * @param maj The major device number for the inode.
 * @param min The minor device number for the inode.
 *
 * @return A pointer to the newly allocated and initialized inode on success.
 * @return NULL if any memory allocation fails during the process.
 */
struct inode* malloc_inode(int mode, unsigned maj, unsigned min)
{
    // Allocate memory for the inode structure itself.
    struct inode *inode = malloc(sizeof(struct inode));
    if (!inode) {
        return NULL;
    }
    
    memset(inode, 0, sizeof(struct inode));

    inode->mode = mode;
    inode->maj = maj;
    inode->min = min;
    
    inode->impl = mcs_mutex_create();
    if (!inode->impl) {
        free(inode);
        return NULL;
    }
    
    if (mode == DIR_MODE) {
        inode->dir = malloc(sizeof(struct dirtb));
        if (!inode->dir) {
            // Cleanup: If `dirtb` allocation fails, destroy the created mutex
            // and free the inode memory before returning.
            mcs_mutex_destroy(inode->impl);
            free(inode);
            return NULL;
        }
        // Zero-initialize the newly allocated directory table.
        memset(inode->dir, 0, sizeof(struct dirtb));
        
        inode->file = NULL;
    } else {
        inode->file = malloc(sizeof(struct indextb));
        if (!inode->file) {
            mcs_mutex_destroy(inode->impl);
            free(inode);
            return NULL;
        }

        memset(inode->file, 0, sizeof(struct indextb));

        inode->dir = NULL;
    }
    
    return inode;
}