[SPECIFICATION]
`encrypt_page`
- Precondition:  `ptr` points to a valid memory region of PAGE_SIZE.
                 `key` points to a valid 16-byte key.
- Postcondition: Encrypts data starting at `ptr` using `key`, use XTEA encryption with a 16-byte key.
                 Alloc a new memory region for the encrypted data, Returns a pointer to the encrypted data.

`decrypt_page`
- Precondition:  `ptr` points to a valid memory region of PAGE_SIZE.
                 `key` points to a valid 16-byte key.
- Postcondition: Decrypts data starting at `ptr` using `key`, use XTEA encryption with a 16-byte key.
                 Alloc a new memory region for the decrypted data, Returns a pointer to the decrypted data.

`free_page`
- Precondition:  `ptr` points to a valid memory region of PAGE_SIZE.
- Postcondition: Frees the memory region pointed to by `ptr`.


[Rely]
```c
// common.h
#define PG_SIZE 4096

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
// encrypt.h
void* encrypt_block(void *ptr, const unsigned char *key);
void* decrypt_block(void *ptr, const unsigned char *key);
void free_block(void *ptr);
```

[PROMPT]
Generate **C code** for data encryption and decryption in `encrypt.c`.
Provide complete `encrypt.h` and `encrypt.c` files that:
- Add helper functions for encryption and decryption if needed. Add new attributes to `inode` if necessary.
- Include necessary headers and define functions as per the specification.
- No libraries other than c std libraries.
