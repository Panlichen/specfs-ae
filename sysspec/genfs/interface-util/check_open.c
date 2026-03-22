#include "interface-util.h"

int check_open(struct inode *inum, unsigned mode) {
    if (inum == NULL) {
        return 1;
    }

    int result;
    if (inum->mode == DIR_MODE) {
        result = (mode == DIR_MODE) ? 0 : 1;
    } else {
        result = (mode != DIR_MODE) ? 0 : 1;
    }

    unlock(inum);
    return result;
}