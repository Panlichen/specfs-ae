#ifndef RBTREE_H
#define RBTREE_H

// Forward declarations
struct rb_node;
struct Prealloc;
struct inode;

// Color enumeration for Red-Black Tree nodes
typedef enum {
    RED,
    BLACK
} rb_color_t;

// Prealloc structure (as defined in common.h)
typedef struct Prealloc {
    unsigned pa_lstart;
    unsigned char *pa_data;
    unsigned pa_len;
    unsigned pa_free;
} Prealloc;

// Red-Black Tree node structure
struct rb_node {
    struct rb_node *parent;
    struct rb_node *left;
    struct rb_node *right;
    rb_color_t color;
    Prealloc *prealloc; // Pointer to the Prealloc structure
};

// Inode structure containing the R-B tree root
typedef struct inode {
    int mutex;
    struct mcs_mutex *impl;
    struct mcs_node *hd;
    unsigned maj;
    unsigned min;
    unsigned int mode;
    unsigned int size;
    struct dirtb* dir;
    void *extents; // Using void* to avoid dependency on Extent struct
    struct rb_node *prealloc_tree; // Red-black tree for prealloc extents
} inode;


/**
 * @brief Finds the maximum node in the tree with a key less than or equal to the target.
 *
 * This function searches the red-black tree to find the node whose `prealloc->pa_lstart`
 * value is the largest one that is still less than or equal to the given target offset.
 *
 * @param root The root node of the red-black tree.
 * @param target The target offset to compare against.
 * @return A pointer to the found rb_node, or NULL if no such node exists.
 */
struct rb_node *rb_find_max_lte(struct rb_node *root, unsigned target);

#endif // RBTREE_H