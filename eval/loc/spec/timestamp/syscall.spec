[PROMPT]
Provide complete functions from the specs.
- Use the provided structures and utilities from [Rely], Use exact function signatures as in [Guarantee].
- Only add the necessary code to support inode timestamps, no other changes.
- No libraries other than c std libraries.

[OPTIMIZATION]
Now generate new C code to update the previous generated C code to support inode timestamps. 

`atomfs_ins`
- PRECONDITION: You are given the path, name, mode, maj and min to insert.
- POSTCONDITION: Locate the directory and insert the file into the directory.
                 Set the file's all timestamps to the current time.
                 Set the dir's access time and modification time to the current time.

`atomfs_del`
- PRECONDITION: You are given the path and name to delete.
- POSTCONDITION: Locate the file and delete the data from the file.
                 Set the dir's access time and modification time to the current time.

`atomfs_readdir`
- PRECONDITION: You are given the path to read.
- POSTCONDITION: Locate the directory and read the data from the directory.
                 Set the dir's access time to the current time.

`atomfs_getattr`
- PRECONDITION: You are given the path to get the attributes.
- POSTCONDITION: Locate the file and get the attributes from the file.
                 Set the file's access time to the current time.

`atomfs_read`
- PRECONDITION: You are given the path, size and offset to read.
- POSTCONDITION: Locate the file and read the data from the file.
                 Set the file's access time to the current time.

`atomfs_write`
- PRECONDITION: You are given the path, buffer, size and offset to write.
- POSTCONDITION: Locate the file and write the data to the file.
                 Set the file's access time and modification time to the current time.

`atomfs_truncate`
- PRECONDITION: You are given the path and offset to truncate.
- POSTCONDITION: Check the boundary of the offset, then locate the file and truncate the data from the file.
                 Set the file's access time and modification time to the current time.

`atomfs_open`
- PRECONDITION: You are given the path and mode to open.
- POSTCONDITION: Locate the file and open the file.
                 Set the file's access time to the current time.

`atomfs_rename`
- PRECONDITION: You are given the source and destination path, source and destination name to rename.
- POSTCONDITION: Locate the source file and rename the file to the destination name.
                 Set the source file's access time and modification time to the current time.
                 Set the two dir's access time and modification time to the current time.

[Rely] Predefined Structures/Functions(do not reimplement)
**Provided Utilities**
```c
//  timestamp.h
#define ACCESS_TIME 0
#define MODIFICATION_TIME 1
#define CHANGE_TIME 2

void set_current_time(struct inode* node, int type);
```

[Guarantee]
**Exposed API**:
```c
//  main_file.h
struct read_ret* atomfs_read(char* path[], unsigned size, unsigned offset);
int atomfs_write(char* path[], const char* buf, unsigned size, unsigned offset);
int atomfs_truncate(char* path[], unsigned offset);
struct inode* atomfs_open(char* path[], unsigned mode);

//  main_dir.h
int atomfs_ins(char* path[], char* name, int mode, unsigned maj, unsigned min);
int atomfs_del(char* path[], char* name);
char** atomfs_readdir(char* path[]);
struct getattr_ret* atomfs_getattr(char* path[]);

//  main_rename.h
int atomfs_rename(char* srcpath[], char* dstpath[], char* srcname, char* dstname);
```

All other functions and structures should be maintained as it is.
