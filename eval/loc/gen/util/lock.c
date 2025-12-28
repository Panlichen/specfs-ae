#include "util.h"

/**
 * @brief Acquires a lock on an inode using the MCS locking protocol.
 *
 *
 * @param[in] inum A non-NULL pointer to a valid `inode` structure. The
 *                 `inum->impl` field is expected to point to an initialized
 *                 MCS lock object.
 */
void lock(struct inode* inum) {
    mcs_node_t* me = malloc(sizeof(mcs_node_t));
    
    mcs_mutex_lock(inum->impl, me);
    inum->hd = me;
    inum->mutex = syscall(SYS_gettid);
}