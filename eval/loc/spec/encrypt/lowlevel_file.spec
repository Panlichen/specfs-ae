[OPTIMIZATION]
Now generate new C code to update the previous generated C code to support data encryption/decryption.

`file_allocate`
- PRECONDITION: You are given an indextb, start offset and length to allocate.
- POSTCONDITION: Allocate the file, encrypt the data per block and write it to the disk.

`file_read`
- PRECONDITION: You are given an indextb, start offset, length to read and a buffer to store the data.
- POSTCONDITION: Read the data from the disk, decrypt it per block and store it in the buffer.

`file_write`
- PRECONDITION: You are given an indextb, start offset, length to write and a buffer with the data.
- POSTCONDITION: Encrypt the data per block, write it to the disk.

[Rely]
Functions and Logic about encryption and decryption are already implemented in the `encrypt.c` file.
You can use the functions from the `encrypt.c` file.
```c
//  common.h
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
    unsigned char key[16];
}inode;

//  encrypt.h
void* encrypt_block(void *ptr, const unsigned char *key);
void* decrypt_block(void *ptr, const unsigned char *key);
void free_block(void *ptr);
```

[Guarantee]
The following functions has to be maintained, you may change the implementation of the functions.
//  lowlevel_file.h
```c
void file_allocate(struct indextb *tb, unsigned offset, unsigned len);
void file_read(struct indextb *tb, unsigned offset, unsigned len, char *data);
void file_write(struct indextb *tb, unsigned offset, unsigned len, const char *data);
```

[PROMPT]
Provide complete `lowlevel_file.h` and `lowlevel_file.c` files according to the above specs.
You may modify the data structure to support encryption/decryption if needed.

