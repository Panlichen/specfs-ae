#ifndef RBTREE_H
#define RBTREE_H

#include "common.h"

typedef enum { RB_RED, RB_BLACK } rb_color_t;

// Exposed API
struct rb_node *rb_find_max_lte(struct rb_node *root, unsigned target);
void rb_delete(struct inode *node, struct rb_node *z);
void rb_insert(struct inode *node, Prealloc *pa);

// Helper functions (internal use)
static struct rb_node *rb_new_node(Prealloc *pa);
static void rb_left_rotate(struct inode *node, struct rb_node *x);
static void rb_right_rotate(struct inode *node, struct rb_node *y);
static void rb_insert_fixup(struct inode *node, struct rb_node *z);
static void rb_transplant(struct inode *node, struct rb_node *u, struct rb_node *v);
static struct rb_node *rb_minimum(struct rb_node *x);
static void rb_delete_fixup(struct inode *node, struct rb_node *x);

#endif // RBTREE_H