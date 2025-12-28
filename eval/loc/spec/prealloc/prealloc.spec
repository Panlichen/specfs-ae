[SPECIFICATION]
`mballoc`
- Precondition: You are given an inode, an start offset, and a length in blocks.
- Postcondition: Alloc len blocks starting at offset in the inode. It will first try to allocate
  from preallocated blocks, and if not enough space is available, it will fall back to
  `malloc_contigous_pages` to allocate the remaining blocks, when doing this, it will first normalize
  to multiples of granularity. If there is extra space, it will create a new prealloc block for
  future allocations.
  The function will create extents for the allocated blocks and update the inode's extent list.

`try_prealloc`
- Precondition: You are given an inode, an start offset, and a length in blocks.
- Postcondition: Try to allocate from preallocated blocks. If some length remains to be allocated,
  it will return the remaining length. The function will iterate over the prealloc list and
  check if the offset falls within the range of any prealloc block. If it does, it will allocate
  the maximum possible space from that block and create an extent for it. If the prealloc block is
  exhausted, it will free it.

`create_extent`
- Precondition: You are given an inode, an start offset, an extent length and a data pointer.
                The extent length is the number of contiguous pages to be allocated.
                The data pointer is the memory address where the data will be stored.
- Postcondition: Create a new extent and append it to the inode's extent list.
                 The function will allocate memory for the new extent and initialize its fields.


[Rely] Predefined Structures (from kernel header "common.h"):
#define PAGE_SIZE 4096      // page size in bytes, one page = one block
#define ALLOC_GRANULARITY 64   // minimum allocation unit

typedef struct Prealloc {
    unsigned pa_lstart;       // logical start block
    unsigned char *pa_data;   // preallocated data buffer
    unsigned pa_len;          // total preallocated blocks
    unsigned pa_free;         // free blocks remaining
    struct Prealloc *next;    // next prealloc extent
} Prealloc;

typedef struct Extent {
    unsigned start_page;   // Logical starting page in file
    unsigned length;       // Number of contiguous pages
    unsigned char *data;   // Contiguous memory for pages
    struct Extent *next;   // Linked list of extents
} Extent;

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
    Prealloc *preallocs;
} inode;

unsigned char* malloc_contigous_pages(unsigned num);
 
[Guarantee]
Exposed API:
// mballoc.h
void mballoc(struct inode *node, unsigned offset, unsigned len); // alloc len blocks starting at offset

[PROMPT]
Provide full `mballoc.c` and `mballoc.h` files that:
1. Implements `mballoc` function as per the specification.
2. Provide necessary helper functions to implement the `mballoc` function.
3. No includes other than the ones specified in the specification and c standard library.
