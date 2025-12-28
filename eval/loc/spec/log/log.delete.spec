[PROMPT]
Provide complete `log.c` files that optimize the original code to support metadata log:
- Implement the `log_delete`, `log_delete_entry`, `log_delete_inode` functions.
- Use the provided structures and utilities from [Rely], Use exact function signatures as in [Guarantee].
- No libraries other than c std libraries.
- Make sure to handle locks in a multithread environment.

[SPECIFICATION]
`log_delete`
- PRECONDITION: You are provided with a reference pointer and its pointed data type.
- POSTCONDITION:  First scan over the log to check if the pointer is already in the log.
                  If it is, simply return.
                  If it is not, call the corresponding delete function to add log for future commit operation.

`log_delete_entry`
- PRECONDITION: You are provided with a pointer to an entry structure.
- POSTCONDITION: Add the entry structure to the delete log. Uniqueness check has been done in `log_delete`.

`log_delete_inode`
- PRECONDITION: You are provided with a pointer to an inode structure.
- POSTCONDITION: Add the inode structure to the delete log. Uniqueness check has been done in `log_delete`.
                 If the inode is a file, add the existing index tables to the delete log
                 via `log_delete_page`. If the inode is a directory, do nothing.

`log_delete_page`
- PRECONDITION: You are provided with a pointer to a page.
- POSTCONDITION: Add the page structure to the delete log. Uniqueness check has been done in `log_delete`.

[Rely] Predefined Structures/Functions(do not reimplement)
**Predefined Structures**
```c
//  log.h
#define LOG_TYPE_DATA 0
#define LOG_TYPE_DIR 1
#define LOG_TYPE_INODE 2

#define FILE_MODE 1
#define DIR_MODE 2

#define PAGE_SIZE 4096

//  common.h
//  data structures of metadata
typedef struct entry{
    char* name;
    void* inum;
    struct entry* next;
}entry;

typedef struct indextb{
    unsigned char *index[INDEXTB_NUM];
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

//  data structures of log
struct log_record {
    void *ref_ptr;
    void *new_ptr;
};

struct write_log {
    int n;
    struct log_record record[MAX_LOG_RECORD_NUM];
};

struct delete_log {
    int n;
    void *ref_ptr[MAX_LOG_RECORD_NUM];
};

struct log {
    int mutex;
    struct mcs_mutex *impl;
    struct mcs_node *hd;
    int readers;
    int writers;
    struct write_log wl[LOG_TYPE_NUM];
    struct delete_log dl[LOG_TYPE_NUM];
};
```

**Provided Utilities**
```c
//  log.h
struct log log;     //  Global log variable

//  util.h
void log_lock(struct log* log);
void log_unlock(struct log* log);

[Guarantee]
**Exposed API**:
```c
// log.h
void log_delete(void* ptr, int type);
```

***Self-contained API**:
```c
// log.h
static void log_delete_entry(struct entry* entry);
static void log_delete_inode(struct inode* inode);
static void log_delete_page(void* page);
```
