[Optimization]
Now generate new C code to update the previous generated C code to support **inline_data** feature for this file system.

[SPECIFICATION of file_allocate]
**Precondition**: there exists an inode 
**Postcondition**: ensure pages exist for [offset, offset + len). If INLINE_DATA_SIZE is in the range [offset, offset + len), it will also allocate `inline_data`.

[SPECIFICATION of file_read]
**Precondition**: there exists an inode 
**Postcondition**: read data from pages. If INLINE_DATA_SIZE is in the range [offset, offset + len), the data from [offset, INLINE_DATA_SIZE) is read from the `inline_data`, [INLINE_DATA_SIZE, offset + len) is read from pages (indextb)

[SPECIFICATION of file_write]
**Precondition**: there exists an inode 
**Postcondition**: write data to pages. If INLINE_DATA_SIZE is in the range [offset, offset + len), the data from [offset, INLINE_DATA_SIZE] is write to the `inline_data`, [INLINE_DATA_SIZE, offset + len) is write to pages (indextb)

[Rely]
#define INLINE_DATA_SIZE = PG_SIZE - 64

typedef struct inode {
    int mutex;
    struct mcs_mutex *impl;
    struct mcs_node *hd;
    unsigned maj;
    unsigned min;
    unsigned int mode;
    unsigned int size;
    struct dirtb* dir;
    struct indextb* file;
    char inline_data[INLINE_DATA_SIZE]; 
} inode;


[PROMPT]
If needed, any modification to the rely data structure or new functions are welcomed.
Provide complete `lowlevel_file.h` and `lowlevel_file.c` files that:
- If you modify a function in [Guarantee], reflect the modified function directly in the file
- If you modify a data structure or function in [Rely], write the new implementation to the files as well. 
- If possible, choose the implementation that does not need to modify the [Rely] and [Guarantee].
- Ensure memory safety.
