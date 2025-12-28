[OPTIMIZATION]
Now generate new C code to update the previous generated C code to support file system logging. 
- Use the provided structures and utilities from [Rely], Use exact function signatures as in [Guarantee].
- No libraries other than c std libraries.

`insert`
- PRECONDITION: You are given the current inode, the inode to be inserted, and the name.
- POSTCONDITION: Create a new entry structure and insert it into the current inode's directory table.
                 You should not modify the current inode in place, but get it from the log and write it
                 to the log.

`delete`
- PRECONDITION: You are given the inode to be deleted and the name.
- POSTCONDITION: Iterate over the current inode's directory table and find the entry with the given name.
                 If found, update the current inode's directory table and free the entry.
                 When modifying the inode, get it from the log and write it to the log.
                 When freeing the entry, directly add it to the delete log.


[Rely] Predefined Structures/Functions(do not reimplement)
**Provided Utilities**
```c
//  log.h
#define LOG_TYPE_DATA 0     //  data log
#define LOG_TYPE_DIR 1      //  entry log
#define LOG_TYPE_INODE 2    //  inode log

void* log_get(void* ptr, int type);     //  get pointer from log of given type
void log_delete(void* ptr, int type);   //  write to delete log, disposal will be done later
```

[Guarantee]
**Exposed API**:
```c
//  directory_operations.h
void insert(struct inode* cur, struct inode* inum, char* name);
struct inode* delete(struct inode* inum, char* name);
```

All other functions and structures should be maintained as it is.

