[OPTIMIZATION]
Now generate new C code to update the previous generated C code to support file system logging. 
- Use the provided structures and utilities from [Rely], Use exact function signatures as in [Guarantee].
- No libraries other than c std libraries.

`inode_write`
- PRECONDITION: You are given the inode, buffer, length, and offset.
- POSTCONDITION: You should first get the inode from the log. Allocate the file if the size is larger.
                 Then write the buffer to the file at target offset.

`inode_truncate`
- PRECONDITION: You are given the inode, and the size.
- POSTCONDITION: You should first get the inode from the log. Allocate the file if the size is larger.
                 Clear if it's shorter.


[Rely] Predefined Structures/Functions(do not reimplement)
**Provided Utilities**
```c
//  log.h
#define LOG_TYPE_DATA 0
#define LOG_TYPE_DIR 1
#define LOG_TYPE_INODE 2

void* log_get(void* ptr, int type);     //  get pointer from log of given type
```

[Guarantee]
**Exposed API**:
```c
//  inode_management.h
unsigned inode_write(struct inode* node, const char* buffer, unsigned len, unsigned offset);
void inode_truncate(struct inode* node, unsigned size);
```

All other functions and structures should be maintained as it is.

