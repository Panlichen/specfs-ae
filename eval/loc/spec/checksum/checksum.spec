[SPECIFICATION]
`checksum_validate`
- Precondition: An inode with a checksum.
- Postcondition: Uses CRC32 for checksum calculation, ignore non-persistent data for checksum calculation.
                 Returns true if the checksum is valid, false otherwise.

`checksum_update`
- Precondition: An inode with a checksum.
- Postcondition: Uses CRC32 for checksum calculation, ignore non-persistent data for checksum calculation.
                 Updates the checksum of the inode.


[Rely]
```c
// common.h
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
}inode;
```

[Guarantee]
**Exposed API**:
```c
// checksum.h
bool checksum_validate(const struct inode *node);
void checksum_update(struct inode *node);
```

[PROMPT]
Generate **C code** for metadata checksum in `checksum.c` and `checksum.h` files.
Provide complete `checksum.h` and `checksum.c` files that:
- Use `inode` from [Rely]. Add new attributes like `checksum` to `inode` if necessary.
- Include necessary headers and define functions as per the specification.
- No libraries other than c std libraries.