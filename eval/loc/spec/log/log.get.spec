[PROMPT]
Provide complete `log.c` files that optimize the original code to support metadata log:
- Implement the `log_get`, `log_copy_entry`, `log_copy_inode`, `log_copy_page` functions.
- Use the provided structures and utilities from [Rely], Use exact function signatures as in [Guarantee].
- No libraries other than c std libraries.
- Make sure to handle locks in a multithread environment.

[SPECIFICATION]
`log_get`
- PRECONDITION: You are provided with a reference pointer and its pointed data type.
- POSTCONDITION: First scan over the log to check if the pointer is already in the log.
                 If it is, return the pointer to the new data.
                 If it is not, call the corresponding copy constructor to create a new data structure. After that,
                 store the pointer in the log.

`log_copy_entry`
- PRECONDITION: You are provided with a pointer to an entry structure.
- POSTCONDITION: Copy the entry structure to a new entry structure and store it in the log.
                 You may use `malloc_entry` for a new entry and `malloc_string` for the name.
                 And shallow copy other fields.

`log_copy_inode`
- PRECONDITION: You are provided with a pointer to an inode structure.
- POSTCONDITION: Copy the inode structure to a new inode structure and store it in the log.
                 You may allocate new memory for the inode, shallow copy mutex related fields,
                 and copy the directory or file tables according to the type of inode(no need for their entry/index tables).

`log_copy_page`
- PRECONDITION: You are provided with a pointer to a page.
- POSTCONDITION: Copy the page structure to a new page structure and store it in the log.

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
unsigned char* malloc_page(); // Allocates zeroed 4KB page
struct entry* malloc_entry(); // Allocates zeroed entry
char* malloc_string(const char* name); // Allocates string with name

void log_lock(struct log* log);
void log_unlock(struct log* log);

[Guarantee]
**Exposed API**:
```c
// log.h
void* log_get(void* ptr, int type);
```

***Self-contained API**:
```c
// log.h
static struct entry* log_copy_entry(struct entry* entry);
static struct inode* log_copy_inode(struct inode* inode);
static unsigned char* log_copy_page(unsigned char* page);
```
