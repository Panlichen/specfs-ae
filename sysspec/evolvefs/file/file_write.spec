[SPECIFICATION]
**Precondition**:
  * `node` is a non-NULL pointer to a valid `inode` structure.
  * The data extents for the range `[offset, offset + len)` have already been allocated by a prior call to `file_allocate`.
  * `data` is a valid pointer to a memory buffer containing at least `len` bytes. If `data` is NULL, the operation will write zeros instead.


**Postcondition**:
  * **Case 1: Successful Write**
      * The content of the file represented by `node` in the range `[offset, offset + len)` is updated with the bytes from the `data` buffer.
      * If the write operation would exceed the maximum file size (`INDEXTB_NUM * PG_SIZE`), the write is truncated at the file size limit.
  * **Case 2: Zero-Fill Write**
      * If `data` is NULL, the content of the file in the specified range is filled with zeros.


[RELY]
```c
#define PG_SIZE 4096
#define INDEXTB_NUM 8192 // Maximum number of pages in the file

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
void file_write(struct inode *node, unsigned offset, unsigned len, const char *data);
```
