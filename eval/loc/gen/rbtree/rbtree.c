#include <stddef.h>
#include <stdlib.h>
#include "rbtree.h"

// Forward declarations for static helper functions
static void rb_left_rotate(struct inode *inode, struct rb_node *x);
static void rb_right_rotate(struct inode *inode, struct rb_node *x);
static void rb_insert_fixup(struct inode *inode, struct rb_node *z);
static void rb_delete_fixup(struct inode *inode, struct rb_node *x);
static void rb_transplant(struct inode *inode, struct rb_node *u, struct rb_node *v);
static struct rb_node* rb_minimum(struct rb_node* node);

/**
 * @brief Finds the maximum node whose prealloc start is <= target.
 *
 * Iteratively searches the tree. If the current node's key is greater than
 * the target, it moves to the left child. If the key is less than or equal
 * to the target, it records the current node as a potential answer and
 * moves to the right child to find a potentially better (larger key) match.
 */
struct rb_node *rb_find_max_lte(struct rb_node *root, unsigned target) {
    struct rb_node *result = NULL;
    struct rb_node *current = root;

    while (current != NULL) {
        if (current->prealloc->pa_lstart > target) {
            // Current node's key is too large, move to the left subtree.
            current = current->left;
        } else {
            // This node is a candidate. Store it and try to find a better
            // one (a larger key that is still <= target) in the right subtree.
            result = current;
            current = current->right;
        }
    }
    return result;
}

/**
 * @brief Performs a left rotation on node x.
 */
static void rb_left_rotate(struct inode *inode, struct rb_node *x) {
    struct rb_node *y = x->right;
    x->right = y->left;

    if (y->left != NULL) {
        y->left->parent = x;
    }
    y->parent = x->parent;

    if (x->parent == NULL) {
        inode->prealloc_tree = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

/**
 * @brief Performs a right rotation on node x.
 */
static void rb_right_rotate(struct inode *inode, struct rb_node *x) {
    struct rb_node *y = x->left;
    x->left = y->right;

    if (y->right != NULL) {
        y->right->parent = x;
    }
    y->parent = x->parent;

    if (x->parent == NULL) {
        inode->prealloc_tree = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }

    y->right = x;
    x->parent = y;
}

/**
 * @brief Inserts a new Prealloc block into the red-black tree.
 *
 * This function allocates a new rb_node, associates it with the given Prealloc
 * data, performs a standard binary search tree insertion, and then calls
 * rb_insert_fixup to restore the red-black tree properties.
 */
static void rb_insert(struct inode *inode, Prealloc *pa) {
    struct rb_node *z = (struct rb_node*) malloc(sizeof(struct rb_node));
    z->prealloc = pa;
    z->left = NULL;
    z->right = NULL;
    z->color = RED; // New nodes are always red

    struct rb_node *y = NULL;
    struct rb_node *x = inode->prealloc_tree;

    // Find the parent for the new node (standard BST insert)
    while (x != NULL) {
        y = x;
        if (z->prealloc->pa_lstart < x->prealloc->pa_lstart) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    z->parent = y;
    if (y == NULL) {
        inode->prealloc_tree = z; // Tree was empty
    } else if (z->prealloc->pa_lstart < y->prealloc->pa_lstart) {
        y->left = z;
    } else {
        y->right = z;
    }

    rb_insert_fixup(inode, z);
}

/**
 * @brief Restores red-black properties after insertion.
 */
static void rb_insert_fixup(struct inode *inode, struct rb_node *z) {
    while (z->parent != NULL && z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            struct rb_node *y = z->parent->parent->right; // Uncle
            if (y != NULL && y->color == RED) {
                // Case 1: Uncle is red
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    // Case 2: Uncle is black and z is a right child
                    z = z->parent;
                    rb_left_rotate(inode, z);
                }
                // Case 3: Uncle is black and z is a left child
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rb_right_rotate(inode, z->parent->parent);
            }
        } else {
            // Symmetric case with "right" and "left" exchanged
            struct rb_node *y = z->parent->parent->left; // Uncle
            if (y != NULL && y->color == RED) {
                // Case 1
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    // Case 2
                    z = z->parent;
                    rb_right_rotate(inode, z);
                }
                // Case 3
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rb_left_rotate(inode, z->parent->parent);
            }
        }
    }
    inode->prealloc_tree->color = BLACK;
}

/**
 * @brief Replaces one subtree as a child of its parent with another subtree.
 */
static void rb_transplant(struct inode *inode, struct rb_node *u, struct rb_node *v) {
    if (u->parent == NULL) {
        inode->prealloc_tree = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    if (v != NULL) {
        v->parent = u->parent;
    }
}

/**
 * @brief Finds the node with the minimum key in a subtree.
 */
static struct rb_node* rb_minimum(struct rb_node* node) {
    while (node->left != NULL) {
        node = node->left;
    }
    return node;
}

/**
 * @brief Deletes a node from the red-black tree.
 *
 * This function handles the removal of a node `z` while maintaining the
 * binary search tree property. It then calls `rb_delete_fixup` if necessary
 * to restore the red-black properties.
 */
static void rb_delete(struct inode *inode, struct rb_node *z) {
    struct rb_node *y = z;
    struct rb_node *x;
    rb_color_t y_original_color = y->color;

    if (z->left == NULL) {
        x = z->right;
        rb_transplant(inode, z, z->right);
    } else if (z->right == NULL) {
        x = z->left;
        rb_transplant(inode, z, z->left);
    } else {
        y = rb_minimum(z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            if (x != NULL) x->parent = y;
        } else {
            rb_transplant(inode, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        rb_transplant(inode, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (y_original_color == BLACK) {
        if (x != NULL) rb_delete_fixup(inode, x);
    }
    
    // Note: The caller is responsible for freeing the `Prealloc` structure if needed.
    // This function only frees the `rb_node`.
    free(z);
}

/**
 * @brief Restores red-black properties after deletion.
 */
static void rb_delete_fixup(struct inode *inode, struct rb_node *x) {
    while (x != inode->prealloc_tree && (x == NULL || x->color == BLACK)) {
        if (x == x->parent->left) {
            struct rb_node *w = x->parent->right; // Sibling
            if (w->color == RED) {
                // Case 1: Sibling w is red
                w->color = BLACK;
                x->parent->color = RED;
                rb_left_rotate(inode, x->parent);
                w = x->parent->right;
            }
            if ((w->left == NULL || w->left->color == BLACK) &&
                (w->right == NULL || w->right->color == BLACK)) {
                // Case 2: Sibling w is black, and both of w's children are black
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right == NULL || w->right->color == BLACK) {
                    // Case 3: Sibling w is black, w's left child is red, right is black
                    if(w->left != NULL) w->left->color = BLACK;
                    w->color = RED;
                    rb_right_rotate(inode, w);
                    w = x->parent->right;
                }
                // Case 4: Sibling w is black, and w's right child is red
                w->color = x->parent->color;
                x->parent->color = BLACK;
                if(w->right != NULL) w->right->color = BLACK;
                rb_left_rotate(inode, x->parent);
                x = inode->prealloc_tree; // Exit loop
            }
        } else {
            // Symmetric case
            struct rb_node *w = x->parent->left; // Sibling
            if (w->color == RED) {
                // Case 1
                w->color = BLACK;
                x->parent->color = RED;
                rb_right_rotate(inode, x->parent);
                w = x->parent->left;
            }
            if ((w->right == NULL || w->right->color == BLACK) &&
                (w->left == NULL || w->left->color == BLACK)) {
                // Case 2
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left == NULL || w->left->color == BLACK) {
                    // Case 3
                    if(w->right != NULL) w->right->color = BLACK;
                    w->color = RED;
                    rb_left_rotate(inode, w);
                    w = x->parent->left;
                }
                // Case 4
                w->color = x->parent->color;
                x->parent->color = BLACK;
                if(w->left != NULL) w->left->color = BLACK;
                rb_right_rotate(inode, x->parent);
                x = inode->prealloc_tree; // Exit loop
            }
        }
    }
    if (x != NULL) x->color = BLACK;
}