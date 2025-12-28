[OPTIMIZATION]
Now generate new C code to update the previous generated C code to support metadata checksum.

`inode_read`
- PRECONDITION: You are given an inode, length to read, and start offset.
- POSTCONDITION: Return actual bytes read. Update the checksum when necessary, validate the
                 checksum when accesing it.

`inode_write`
- PRECONDITION: You are given an inode, write buffer with write length, and start offset.
- POSTCONDITION: Write the data to the file. Update the checksum when necessary, validate the
                 checksum when accesing it.

`inode_truncate`
- PRECONDITION: You are given an inode, and its truncated size.
- POSTCONDITION: Truncate the file to the target size. Update the checksum when necessary,
                 validate the checksum when accesing it.

[Rely]
Functions and Logic about metadata checksum are already implemented in the `checksum.c` file.
You can use the functions from the `checksum.c` file.
```c
//  common.h
typedef struct inode{
    int mutex;
    struct mcs_mutex *impl;
    struct mcs_node *hd;
    unsigned maj;
    unsigned min;
    unsigned int mode;
    unsigned int size;
    struct dirtb* dir;
    struct indextb* file;
    uint32_t checksum;
}inode;

//  checksum.h
bool checksum_validate(const struct inode *node);
void checksum_update(struct inode *node);
```

[Guarantee]
The following functions has to be maintained, you may change the implementation of the functions.
//  inode_management.c
```c
struct read_ret* inode_read(struct inode* node, unsigned len, unsigned offset);
unsigned inode_write(struct inode* node, const char* buffer, unsigned len, unsigned offset);
void inode_truncate(struct inode* node, unsigned size);
```

All other functions and structures should be maintained as it is.
