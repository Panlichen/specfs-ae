[OPTIMIZATION]
Now generate new C code to update the previous generated C code to support metadata checksum. 

`file_allocate`
- PRECONDITION: You are given an indextb, start offset and length to allocate.
- POSTCONDITION: Allocate the file, update the checksum when necessary, validate the checksum
                 when accesing it.

`file_read`
- PRECONDITION: You are given an indextb, start offset, length to read and a buffer to store the data.
- POSTCONDITION: Read the data from the file, update the checksum when necessary, validate the
                 checksum when accesing it.

`file_write`
- PRECONDITION: You are given an indextb, start offset, length to write and a buffer with the data.
- POSTCONDITION: Write the data to the file, update the checksum when necessary, validate the
                 checksum when accesing it.

[Rely]
Functions and Logic about metadata checksum are already implemented in the `checksum.c` file.
You can use the functions from the `checksum.c` file.
```c
//  common.h
typedef struct entry{
    char* name;
    void* inum;
    struct entry* next;
}entry;

typedef struct indextb{
    unsigned char *index[INDEXTB_NUM];
    struct inode *parent_inode;
}indextb;

typedef struct dirtb{
    struct entry* tb[DIRTB_NUM];
}dirtb;

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
//  lowlevel_file.c
```c
void file_allocate(struct indextb *tb, unsigned offset, unsigned len);
void file_read(struct indextb *tb, unsigned offset, unsigned len, char *data);
void file_write(struct indextb *tb, unsigned offset, unsigned len, const char *data);
```

All other functions and structures should be maintained as it is.
