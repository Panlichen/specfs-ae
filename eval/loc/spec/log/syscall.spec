[OPTIMIZATION]
Now generate new C code to update the previous generated C code to support file system logging. 
- Use the provided structures and utilities from [Rely], Use exact function signatures as in [Guarantee].
- No libraries other than c std libraries.
- All locks have been handled correctly in the previous code, so no need to handle them again.

`atomfs_rename`
- PRECONDITION: You are given the src and dst paths and names.
- POSTCONDITION: The src file is deleted and the dst file is created with the same content as the src file.
                 See the whole function as a transaction. You should not dispose the destination inode at this time,
                 but write it to the delete log.

`atomfs_open`
- PRECONDITION: You are given the path and mode.
- POSTCONDITION: The inode is opened according to the mode.
                 See the whole function as a transaction.

`atomfs_ins`
- PRECONDITION: You are given the path, name, mode, maj, and min.
- POSTCONDITION: The inode is inserted into the directory.
                 See the whole function as a transaction.

`atomfs_del`
- PRECONDITION: You are given the path and name.
- POSTCONDITION: The inode is deleted from the directory.
                 See the whole function as a transaction. You should not dispose the inode at this time,
                 but write it to the delete log.

`atomfs_readdir`
- PRECONDITION: You are given the path.
- POSTCONDITION: The directory is read and the entries are returned.
                 See the whole function as a transaction.

`atomfs_getattr`
- PRECONDITION: You are given the path.
- POSTCONDITION: The attributes of the inode are returned.
                 See the whole function as a transaction.

[Rely] Predefined Structures/Functions(do not reimplement)
**Provided Utilities**
```c
//  log.h
#define READ_MODE 0
#define WRITE_MODE 1

#define LOG_TYPE_DATA 0
#define LOG_TYPE_DIR 1
#define LOG_TYPE_INODE 2

void begin_trans(int mode);
void end_trans(int mode);

void log_delete(void* ptr, int type);   //  write to delete log, disposal will be done later
```

[Guarantee]
**Exposed API**:
```c
//  main_rename.h
int atomfs_rename(char* srcpath[], char* dstpath[], char* srcname, char* dstname);

//  main_file.h
struct inode* atomfs_open(char* path[], unsigned mode);

//  main_dir.h
int atomfs_ins(char* path[], char* name, int mode, unsigned maj, unsigned min);
int atomfs_del(char* path[], char* name);
char** atomfs_readdir(char* path[]);
struct getattr_ret* atomfs_getattr(char* path[]);
```

All other functions and structures should be maintained as it is.

