[SPECIFICATION]
**Precondition**:
  * `node` is a non-NULL pointer to a valid `inode` structure.
  * `offset` and `len` define the byte range within the file to be allocated.

**Postcondition**:
  * **Case 1 (Invalid Request)**: If `len` is `0` or `offset + len` exceeds `MAX_FILE_SIZE`, the function has no effect and the inode's state remains unchanged.
  * **Case 2 (Successful Allocation)**:
      * The page range corresponding to the byte range `[offset, offset + len)` is determined.
      * For every contiguous block of pages within this range that was not previously allocated, a new `Extent` is created.
      * Each new `Extent` structure is fully initialized, and its `data` field points to newly allocated physical memory.
      * All newly created `Extent`s are inserted at the head of the `node->extents` linked list.
      * Pages that were already allocated within the range are left untouched.

**System Algorithm**:

1.  **Input Validation**: Check if `len` is 0 or `offset + len > MAX_FILE_SIZE`. If so, return immediately.
2.  **Page Calculation**: Convert the byte-based `offset` and `len` into page-based range `start_page` and `end_page`.
3.  **Iterate and Allocate**:
      * Initialize a `current` page counter to `start_page`.
      * Loop while `current <= end_page`.
      * **Internal Allocation Check**: Determine if `current` is already allocated by iterating through `node->extents`. If for any extent, `current >= extent->start_page && current < extent->start_page + extent->length`, the page is considered allocated.
      * If the **Internal Allocation Check** is true, increment `current` and continue to the next iteration.
      * If the page is not allocated:
        a.  Mark the start of a contiguous block of free pages (`block_start = current`).
        b.  Start a nested loop to find the end of this block. Keep incrementing `current` as long as `current <= end_page` and the **Internal Allocation Check** for the `current` page is false.
        c.  Calculate the block length: `block_length = current - block_start`.
        d.  Create a new `Extent`: Allocate physical memory via `malloc_contiguous_pages`.
        iv.  Insert the new extent at the head of the list: `ext->next = node->extents; node->extents = ext;`.
4.  The main loop will naturally resume at the new `current` position, which is the first page after the newly allocated block.

[RELY]
```c
#define PG_SIZE 4096
#define INDEXTB_NUM 8192
#define DIRTB_NUM 512
#define MAX_FILE_LEN 256
#define MAX_DIR_SIZE 10000000
#define MAX_PATH_LEN 100
#define MAX_FILE_SIZE ((unsigned)(INDEXTB_NUM * PG_SIZE))

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

unsigned char* malloc_contiguous_pages(unsigned num);
```

[GUARANTEE]
```c
void file_allocate(struct inode *node, unsigned offset, unsigned len);
```
