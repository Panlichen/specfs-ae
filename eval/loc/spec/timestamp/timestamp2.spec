[SPECIFICATION]
`set_current_time`
- PRECONDITION: You are given a pointer to an inode structure and an time type.
- POSTCONDITION: Set the current time to the inode structure based on the time type.

`get_current_time`
- PRECONDITION: You are given a pointer to an inode structure and an time type.
- POSTCONDITION: Get the current time from the inode structure based on the time type.
                 Return the time in a timespec structure.

[Rely] Predefined Structures/Functions(do not reimplement)
**Predefined Structures**
```c
//  timestamp.h
#define ACCESS_TIME 0
#define MODIFICATION_TIME 1
#define CHANGE_TIME 2

//  common.h
typedef struct inode{
    int mutex;
    struct mcs_mutex *impl;
    struct mcs_node *hd;
    unsigned maj;
    unsigned min;
    unsigned int mode;
    unsigned int size;
    uint32_t atime;
    uint32_t mtime;
    uint32_t ctime;
    struct dirtb* dir;
    struct indextb* file;
}inode;

//  time.h
struct timespec {
  __time_t tv_sec;		/* Seconds.  */
  __syscall_slong_t tv_nsec;	/* Nanoseconds.  */
};

**Provided Utilities**
//  time.h
time_t time (time_t *__timer);

[Guarantee]
**Exposed API**:
```c
// timestamp.h
void set_current_time(struct inode* node, int type);
struct timespec get_current_time(struct inode* node, int type);
```

[PROMPT]
Provide complete `timestamp.h` and `timestamp.c` files that:
- Implement the `set_current_time` function.
- Use the provided structures and utilities from [Rely], Use exact function signatures as in [Guarantee].
- No libraries other than c std libraries.
