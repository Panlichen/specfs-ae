[PROMPT]
Provide complete functions from the specs.
- Use the provided structures and utilities from [Rely], Use exact function signatures as in [Guarantee].
- No libraries other than c std libraries.

[OPTIMIZATION]
Now generate new C code to update the previous generated C code to support inode timestamps with higher
accuracy, e.g. nsec. 

`set_current_time`
- PRECONDITION: You are given a pointer to an inode structure and an time type.
- POSTCONDITION: Set the current time to the inode structure based on the time type.
                 To ensure compatibility, we added a 32-bit ⁠extra field alongside the original ⁠time field.
                 The lower 2 bits extend the original timestamp, while the upper 30 bits represent nanosecond counts.
                 You need to combine these two fields correctly to set the current time.

`get_current_time`
- PRECONDITION: You are given a pointer to an inode structure and an time type.
- POSTCONDITION: Get the current time from the inode structure based on the time type.
                 You need to calculate the current time using the original timestamp and the extra field.
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
    uint32_t atime_extra;
    uint32_t mtime_extra;
    uint32_t ctime_extra;
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
int clock_gettime(clockid_t __clock_id, struct timespec *__tp);

[Guarantee]
**Exposed API**:
```c
// timestamp.h
void set_current_time(struct inode* node, int type);
struct timespec get_current_time(struct inode* node, int type);
```

All other functions and structures should be maintained as it is.
