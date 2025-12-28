[RELY]
```c
typedef struct inode {
	int mutex;
	struct mcs_mutex *impl;
	struct mcs_node *hd;
	unsigned maj;
	unsigned min;
	unsigned int mode;
	unsigned int size;
	struct dirtb *dir;
	Extent *extents;
} inode;

typedef struct Extent {
    unsigned start_page;
    unsigned length;
    unsigned char *data;
    struct Extent *next;
} Extent;
```

[GUARANTEE]
```c
void dispose_inode(struct inode* inum);
```

[SPECIFICATION]
**Pre-Condition**:
- Follow the previous specification.

**Post-Condition**:
- Follow the previous specification.


[OPTIMIZATION]
- the `file` structure is replaced by an `extent` structure. If the inode is a file, free all the extents of the inode.
