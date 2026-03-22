#include "util.h"

void lock(struct inode* inum) {
    // Allocate a new MCS node for this thread
    mcs_node_t* me = malloc(sizeof(mcs_node_t));
    
    // Acquire the lock using MCS algorithm
    mcs_mutex_lock(inum->impl, me);
    
    // Update inode metadata to record lock ownership
    inum->hd = me;                         // Set head pointer to this thread's node
    inum->mutex = syscall(SYS_gettid);     // Set owner to current thread ID
}