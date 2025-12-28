[OPTIMIZATION]
Now generate new C code to update the previous generated C code to support red-black tree traversal of prealloc nodes.

`mballoc`
- Precondition: You are given an inode, an start offset, and a length in blocks.
- Postcondition: Alloc len blocks starting at offset in the inode. Use rbtree functions to update the prealloc list, note that prealloc ranges are not overlapping.

`try_prealloc`
- Precondition: You are given an inode, an start offset, and a length in blocks.
- Postcondition: Try to allocate from preallocated blocks. First find the maximum prealloc node whose start offset is less than or equal to the target offset.
                 If the offset is within the prealloc node range, allocate from it and update the prealloc node. Then if the prealloc node is exhausted, free it and remove it from the red-black tree.
                 Return the remaining length to be allocated.

[Rely]
Following data structures are provided to you, and you can directly use them in your implementation:
```c
//  Predefined Structures (from header "common.h")
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

//  Predefined Functions (from header "rbtree.h")
struct rb_node *rb_find_max_lte(struct rb_node *root, unsigned target); //  Find the maximum node whose start offset less than or equal to target
static void rb_delete(struct inode *node, struct rb_node *z);   //  Delete a node from the red-black tree
static void rb_insert(struct inode *node, Prealloc *pa);    //  Insert a prealloc node into the red-black tree
```

[Guarantee]
```c
void mballoc(struct inode *node, unsigned offset, unsigned len);
```

[PROMPT]
Provide complete `mballoc.h` and `mballoc.c` files that:
- You should not modify the existing structures and functions in [Rely] and [Guarantee].
- Implement the `mballoc` and `try_prealloc` functions to use red-black tree traversal for prealloc nodes.
- Ensure memory safety.
- You may not use any external libraries expect for the standard C library.
