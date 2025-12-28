[SPECIFICATION]
**Precondition**:
  - `node` is a non-NULL pointer to a valid `inode` structure.
  - `data` is a non-NULL pointer to a memory buffer of at least `len` bytes in size.
  - `offset` and `len` specify the starting byte and the total number of bytes to read from the file.

**Postcondition**:
  - The `data` buffer is populated with `len` bytes read from the file starting at `offset`.
  - **Case 1 (Data in Extent)**: For any portion of the read range that maps to a valid, allocated extent, the corresponding data from that extent is copied into the buffer.
  - **Case 2 (Unallocated Data or Out of Bounds)**: For any portion of the read range that corresponds to an unallocated page (a "hole") or is beyond the maximum page limit (`INDEXTB_NUM`), the corresponding bytes in the `data` buffer are filled with zeros.

[RELY]
```c
#define PG_SIZE 4096      // Size of a page in bytes
#define INDEXTB_NUM 8192  // Maximum number of pages in a file

typedef struct Extent {
    unsigned start_page;   // Logical starting page in file
    unsigned length;       // Number of contiguous pages
    unsigned char *data;   // Contiguous memory for pages
    struct Extent *next;   // Linked list of extents
} Extent;

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
void file_read(struct inode *node, unsigned offset, unsigned len, char *data);
```
