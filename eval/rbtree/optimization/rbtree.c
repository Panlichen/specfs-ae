#include "rbtree.h"
#include <stdlib.h>

extern int pool_access_count;

// Create a new red-black tree node
static struct rb_node *rb_new_node(Prealloc *pa) {
    struct rb_node *node = (struct rb_node *)malloc(sizeof(struct rb_node));
    if (!node) return NULL;
    
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    node->color = RB_RED; // New nodes are always red
    node->prealloc = pa;
    return node;
}

// Left rotation
static void rb_left_rotate(struct inode *node, struct rb_node *x) {
    pool_access_count += 2;
    struct rb_node *y = x->right;
    x->right = y->left;
    
    if (y->left != NULL)
        y->left->parent = x;
    
    y->parent = x->parent;
    
    if (x->parent == NULL)
        node->prealloc_tree = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    
    y->left = x;
    x->parent = y;
}

// Right rotation
static void rb_right_rotate(struct inode *node, struct rb_node *y) {
    pool_access_count += 2;
    struct rb_node *x = y->left;
    y->left = x->right;
    
    if (x->right != NULL)
        x->right->parent = y;
    
    x->parent = y->parent;
    
    if (y->parent == NULL)
        node->prealloc_tree = x;
    else if (y == y->parent->right)
        y->parent->right = x;
    else
        y->parent->left = x;
    
    x->right = y;
    y->parent = x;
}

// Find the maximum node with prealloc start <= target
struct rb_node *rb_find_max_lte(struct rb_node *root, unsigned target) {
    struct rb_node *current = root;
    struct rb_node *result = NULL;
    
    while (current != NULL) {
        pool_access_count++;
        if (current->prealloc->pa_lstart == target) {
            return current;
        } else if (current->prealloc->pa_lstart < target) {
            result = current;
            current = current->right;
        } else {
            current = current->left;
        }
    }
    
    return result;
}

// Insert a new node into the tree
void rb_insert(struct inode *node, Prealloc *pa) {
    struct rb_node *z = rb_new_node(pa);
    if (!z) return;
    
    struct rb_node *y = NULL;
    struct rb_node *x = node->prealloc_tree;
    
    while (x != NULL) {
        pool_access_count++;
        y = x;
        if (z->prealloc->pa_lstart < x->prealloc->pa_lstart)
            x = x->left;
        else
            x = x->right;
    }
    
    pool_access_count++;
    z->parent = y;
    if (y == NULL)
        node->prealloc_tree = z;
    else if (z->prealloc->pa_lstart < y->prealloc->pa_lstart)
        y->left = z;
    else
        y->right = z;
    
    rb_insert_fixup(node, z);
}

// Fix red-black properties after insertion
static void rb_insert_fixup(struct inode *node, struct rb_node *z) {
    while (z->parent != NULL && z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            struct rb_node *y = z->parent->parent->right;
            if (y != NULL && y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rb_left_rotate(node, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rb_right_rotate(node, z->parent->parent);
            }
        } else {
            struct rb_node *y = z->parent->parent->left;
            if (y != NULL && y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rb_right_rotate(node, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rb_left_rotate(node, z->parent->parent);
            }
        }
        pool_access_count += 2;
    }
    node->prealloc_tree->color = RB_BLACK;
}

// Replace subtree u with subtree v
static void rb_transplant(struct inode *node, struct rb_node *u, struct rb_node *v) {
    if (u->parent == NULL)
        node->prealloc_tree = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;
    
    if (v != NULL)
        v->parent = u->parent;
}

// Find the minimum node in a subtree
static struct rb_node *rb_minimum(struct rb_node *x) {
    while (x->left != NULL)
        x = x->left;
    return x;
}

// Delete a node from the tree
void rb_delete(struct inode *node, struct rb_node *z) {
    struct rb_node *y = z;
    struct rb_node *x;
    rb_color_t y_original_color = y->color;
    
    if (z->left == NULL) {
        x = z->right;
        rb_transplant(node, z, z->right);
    } else if (z->right == NULL) {
        x = z->left;
        rb_transplant(node, z, z->left);
    } else {
        y = rb_minimum(z->right);
        y_original_color = y->color;
        x = y->right;
        
        if (y->parent == z) {
            if (x != NULL)
                x->parent = y;
        } else {
            rb_transplant(node, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        
        rb_transplant(node, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }
    
    if (y_original_color == RB_BLACK && x != NULL)
        rb_delete_fixup(node, x);
    
    free(z);
}

// Fix red-black properties after deletion
static void rb_delete_fixup(struct inode *node, struct rb_node *x) {
    while (x != node->prealloc_tree && (x == NULL || x->color == RB_BLACK)) {
        if (x == x->parent->left) {
            struct rb_node *w = x->parent->right;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                rb_left_rotate(node, x->parent);
                w = x->parent->right;
            }
            if ((w->left == NULL || w->left->color == RB_BLACK) &&
                (w->right == NULL || w->right->color == RB_BLACK)) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->right == NULL || w->right->color == RB_BLACK) {
                    if (w->left != NULL)
                        w->left->color = RB_BLACK;
                    w->color = RB_RED;
                    rb_right_rotate(node, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                if (w->right != NULL)
                    w->right->color = RB_BLACK;
                rb_left_rotate(node, x->parent);
                x = node->prealloc_tree;
            }
        } else {
            struct rb_node *w = x->parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                rb_right_rotate(node, x->parent);
                w = x->parent->left;
            }
            if ((w->right == NULL || w->right->color == RB_BLACK) &&
                (w->left == NULL || w->left->color == RB_BLACK)) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->left == NULL || w->left->color == RB_BLACK) {
                    if (w->right != NULL)
                        w->right->color = RB_BLACK;
                    w->color = RB_RED;
                    rb_left_rotate(node, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                if (w->left != NULL)
                    w->left->color = RB_BLACK;
                rb_right_rotate(node, x->parent);
                x = node->prealloc_tree;
            }
        }
    }
    if (x != NULL)
        x->color = RB_BLACK;
}