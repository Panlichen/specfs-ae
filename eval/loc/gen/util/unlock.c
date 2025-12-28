#include "util.h"

/**
 * @brief Releases the MCS lock associated with a given inode.
 *
 * @param[in] inum A pointer to the inode structure whose lock is to be released.
 *
 */
void unlock(struct inode* inum) {
    mcs_node_t* me = inum->hd;

    inum->mutex = 0;

    mcs_mutex_unlock(inum->impl, me);

    free(me);
}