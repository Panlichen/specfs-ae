[SPECIFICATION]
Implement red-black tree operations for a memory allocator.

`rb_find_max_lte`
- PRECONDITION: You are given a root node and a target value.
- POSTCONDITION: Find the maximum node in the red-black tree whose prealloc start is less than or equal to the target offset.

`rb_delete`
- PRECONDITION: You are given a node and a root.
- POSTCONDITION: Remove the node from the red-black tree and maintain the properties.

`rb_insert`
- PRECONDITION: You are given a node and a root.
- POSTCONDITION: Insert the node into the red-black tree and maintain the properties.

[Rely]
**Predefined Structures** (from `common.h`):
```c
// Prealloc structure
typedef struct Prealloc {
    unsigned pa_lstart;
    unsigned char *pa_data;
    unsigned pa_len;
    unsigned pa_free;
} Prealloc;

// Newly added red-black tree node structure
struct rb_node {
    struct rb_node *parent;
    struct rb_node *left;
    struct rb_node *right;
    rb_color_t color;
    Prealloc *prealloc; // Pointer to the Prealloc structure
};

//  The followings are modifications to the [Rely]
typedef struct inode{
    int mutex;
    struct mcs_mutex *impl;
    struct mcs_node *hd;
    unsigned maj;
    unsigned min;
    unsigned int mode;
    unsigned int size;
    struct dirtb* dir;
    Extent *extents;       // Extent list head
    struct rb_node *prealloc_tree;   // Red-black tree for prealloc extents
} inode;
```

[Guarantee]
**Exposed API**:
```c
// rbtree.h
struct rb_node *rb_find_max_lte(struct rb_node *root, unsigned target);
static void rb_delete(struct inode *node, struct rb_node *z);
static void rb_insert(struct inode *node, Prealloc *pa);
```

[PROMPT]
Generate **C code** for the red-black tree operations, provide complete `rbtree.h` and `rbtree.c` files that:
- Define the red-black tree operations.
- Implement the exposed API functions.
- Include necessary headers and define any helper functions if you want.
