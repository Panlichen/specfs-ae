[PROMPT]
Provide complete `log.c` files that optimize the original code to support metadata log:
- Implement the `log_commit`, `flush_log`, `flush_log_entry`, `flush_log_page`, `flush_log_inode` functions.
- Use the provided structures and utilities from [Rely], Use exact function signatures as in [Guarantee].
- No libraries other than c std libraries.
- Carefully handle the memory management and ensure that all pointers are freed correctly.

[SPECIFICATION]
`log_commit`
- PRECONDITION: `log` is provided as a global variable.
- POSTCONDITION: check if `log` is empty. If not, call `flush_log` to write the changes.

`flush_log_entry`
- PRECONDITION: You are provided with a pointer to a reference entry and its new entry.
- POSTCONDITION: Two cases. If the new entry is NULL, directly free the reference entry.
                 If the new entry is not NULL, copy the new entry to the reference entry and free the new entry.
                 The entry and its name all need to be freed.

`flush_log_page`
- PRECONDITION: You are provided with a pointer to a reference page and its new page.
- POSTCONDITION: Two cases. If the new page is NULL, directly free the reference page.
                 If the new page is not NULL, copy the new page to the reference page and free the new page.

`flush_log_inode`
- PRECONDITION: You are provided with a pointer to a reference inode and its new inode.
- POSTCONDITION: Two cases. If the new inode is NULL, directly free the reference inode.
                 If the new inode is not NULL, copy the new inode to the reference inode and free the new inode.
                 When free or copy, make sure to free the directory or file tables according to the type of inode,
                 but operations on their entry/index tables are not needed. Shallow copy mutex related fields.

`flush_log`
- PRECONDITION: `log` is provided as a global variable.
- POSTCONDITION: `log` is flushed, and all changes are written to the disk. Iterate through `write_log` and `delete_log`
                 to write the changes and free the memory.

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

//  mcs.h
int mcs_mutex_destroy(mcs_mutex_t *lock);

[Guarantee]
***Self-contained API**:
```c
// log.h
static void log_commit();
static void flush_log();
static void flush_log_entry(struct entry* ref, struct entry* new);
static void flush_log_page(unsigned char* ref, unsigned char* new);
static void flush_log_inode(struct inode* ref, struct inode* new);
```
