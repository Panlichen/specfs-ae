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
```

[GUARANTEE]
```c
struct inode* malloc_inode(int mode, unsigned maj, unsigned min);
```

[SPECIFICATION]
**Pre-Condition**:
- Follow the previous specification.

**Post-Condition**:
- Follow the previous specification.

[OPTIMIZATION]
- `file` field of `inode` structure is replaced by an `extent` structure, and you only have to set it NULL.