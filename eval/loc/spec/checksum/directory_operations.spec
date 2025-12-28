[OPTIMIZATION]
Now generate new C code to update the previous generated C code to support metadata checksum.

[SPECIFICATION]
`insert`
- PRECONDITION: You are given a directory inode, file inode and its name.
- POSTCONDITION: Insert the file into the dir, update the checksum when necessary, validate the
                 checksum when accesing it.

`delete`
- PRECONDITION: You are given a directory inode and a file name.
- POSTCONDITION: Delete the file in that dir, update the checksum when necessary, validate the
                 checksum when accessing it.

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
//  directory_operations.c
```c
struct inode* delete(struct inode* inum, char* name);
void insert(struct inode* cur, struct inode* inum, char* name);
```

All other functions and structures should be maintained as it is.

[PROMPT]
Provide complete `directory_operations.c` file from the specs.

