#include "interface.h"

char **atomfs_readdir(char *path[])
{
    // Lock the root inode before starting traversal
    lock(root_inum);
    
    // Locate the target inode starting from root
    struct inode *target = locate(root_inum, path);
    
    // If locate returned NULL, path resolution failed and all locks are released
    if (target == NULL) {
        return NULL;
    }
    
    // Check if target is a directory (target is locked at this point)
    if (check_dir(target) != 0) {
        // check_dir released the lock on target on failure
        return NULL;
    }
    
    // Allocate directory content array (size + 1 for NULL termination)
    char **dir_content = malloc_dir_content(target->size);
    if (dir_content == NULL) {
        // Allocation failed - release the lock on target
        unlock(target);
        return NULL;
    }
    
    // Fill the directory content (target remains locked during fill_dir)
    fill_dir(target, dir_content);
    
    // Release the lock on target before returning
    unlock(target);
    
    return dir_content;
}