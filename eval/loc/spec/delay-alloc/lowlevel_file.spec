[PROMPT]
Provide complete `lowlevel_file.c` file from the specs.
- Use the provided structures and utilities from [Rely], Use exact function signatures as in [Guarantee].
- No libraries other than c std libraries.

[OPTIMIZATION]
Now generate new C code to update the previous generated C code to support delayed data allocation. 

`file_allocate`
- PRECONDITION: You are given an indextb, start offset, length to allocate.
- POSTCONDITION: Do nothing. Since the allocation is delayed, the function does not need to do anything.

`file_read`
- PRECONDITION: You are given an indextb, start offset, length to read and a buffer to store the data.
- POSTCONDITION: The function reads the data from the file at the specified offset.
                 Always use bread to read one page. It will return NULL if the page is not allocated, you can zero fill the target space.
                 Otherwise, copy the data from the page to the buffer.

`file_write`
- PRECONDITION: You are given an indextb, start offset, length to write and a buffer with the data.
- POSTCONDITION: The function writes the data to the file at the specified offset.
                 Always use bwrite to write the data back. Passing NULL as the data pointer will zero fill the target space.

[Rely] Predefined Structures/Functions(do not reimplement)
**Predefined Structures**
```c
//  common.h
#define PAGE_SIZE 4096

**Provided Utilities**
```c
//  delalloc.h
//  get one page from the buffer
unsigned char *bread(struct indextb *tb, unsigned int page);
//  write one page to the buffer, if buf is NULL, zero fill the target space
void bwrite(struct indextb *tb, unsigned int page, unsigned int offset, unsigned char *buf, unsigned int size);
```

[Guarantee]
**Exposed API**:
```c
//  lowlevel_file.c
void file_allocate(struct indextb *tb, unsigned offset, unsigned len);
void file_read(struct indextb *tb, unsigned offset, unsigned len, char *data);
void file_write(struct indextb *tb, unsigned offset, unsigned len, const char *data);
```

All other functions and structures should be maintained as it is.
