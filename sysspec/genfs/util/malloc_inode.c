#include "util.h"

struct inode* malloc_inode(int mode, unsigned maj, unsigned min)
{
    struct inode *node = (struct inode *)malloc(sizeof(struct inode));
    if (!node) return NULL;

    // Zero-initialize entire inode structure
    memset(node, 0, sizeof(struct inode));

    // Set basic fields from arguments
    node->mode = mode;
    node->maj = maj;
    node->min = min;

    // Initialize MCS mutex
    node->impl = mcs_mutex_create();
    if (!node->impl) {
        free(node);
        return NULL;
    }

    // Initialize based on mode
    if (mode == DIR_MODE) {
        node->dir = (struct dirtb *)malloc(sizeof(struct dirtb));
        if (!node->dir) {
            free(node->impl);
            free(node);
            return NULL;
        }
        memset(node->dir, 0, sizeof(struct dirtb));
        node->file = NULL;
    } else {
        node->file = (struct indextb *)malloc(sizeof(struct indextb));
        if (!node->file) {
            free(node->impl);
            free(node);
            return NULL;
        }
        memset(node->file, 0, sizeof(struct indextb));
        node->dir = NULL;
    }

    return node;
}