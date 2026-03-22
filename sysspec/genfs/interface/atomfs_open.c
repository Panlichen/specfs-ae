#include "interface.h"

struct inode *atomfs_open(char *path[], unsigned mode) {
    // Lock root_inum to satisfy locate's pre-condition
    lock(root_inum);
    
    // Traverse the path from root
    struct inode *target = locate(root_inum, path);
    
    // If path traversal failed, unlock root_inum and return NULL
    if (target == NULL) {
        unlock(root_inum);
        return NULL;
    }
    
    // Check access mode (check_open will unlock target)
    int check_result = check_open(target, mode);
    
    // Return target if check passed, NULL otherwise
    return (check_result == 0) ? target : NULL;
}