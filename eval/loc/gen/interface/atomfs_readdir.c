#include "interface.h"

/**
 * @brief Reads the contents of a directory specified by a path.
 *
 * This function traverses the file system starting from the `root_inum` to find the
 * directory specified by `path`. If the path resolves to a valid directory, it
 * allocates and returns a NULL-terminated array of strings containing the names of
 * the directory entries.
 *
 * @param path A NULL-terminated array of strings representing the path to the directory.
 *             This is a pre-condition for the function.
 *
 * @return On success (Case 1), returns a dynamically allocated, NULL-terminated
 *         array of strings containing the directory entries. The caller is responsible
 *         for freeing this memory.
 * @return On failure (Case 2), returns NULL if:
 *         a) The path does not resolve to a valid inode (`locate` returns NULL).
 *         b) The target inode is not a directory (`check_dir` fails).
 *         c) Memory allocation for the directory content fails.
 */
char **atomfs_readdir(char *path[])
{
    // Per the [SPECIFICATION of atomfs_readdir], no lock is held at entry.
    // Lock the root inode, as `locate` requires its first argument to be locked.
    lock(root_inum);

    // Attempt to resolve the path. Per [SPECIFICATION of locate], `locate` will
    // release the lock on `root_inum` and acquire a lock on `target` if found.
    struct inode *target = locate(root_inum, path);
    
    // Case 2a: Path traversal failed.
    if (target == NULL) {
        // As per [SPECIFICATION of locate], if it returns NULL, all locks
        // have been released. It's safe to return immediately.
        return NULL;
    }
    
    // At this point, `target` is non-NULL and its lock is held.
    // Case 2b: The located inode is not a directory.
    if (check_dir(target) != 0) {
        // The pre-condition for `check_dir` (target is locked) is met.
        // As per [SPECIFICATION of check_dir], on failure (non-zero return),
        // the lock on `target` is released. It's safe to return.
        return NULL;
    }
    
    // Case 1: Successful read. The target is a valid directory.
    // As per [SPECIFICATION of check_dir], on success (return 0), the lock
    // on `target` is still held.
    
    // Allocate an array of strings for the directory content.
    // The size is `target->size + 1` to accommodate the NULL terminator,
    // as required by the Post-Condition.
    char **dir_content = malloc_dir_content(target->size + 1);
    
    // Handle memory allocation failure.
    if (dir_content == NULL) {
        // The lock on `target` is still held, so we must release it
        // before returning to meet the function's post-condition.
        unlock(target);
        return NULL;
    }
    
    // Populate the allocated array with the names of the directory entries.
    fill_dir(target, dir_content);
    
    // Release the lock on the target directory before returning.
    // This fulfills the function's post-condition of holding no locks on exit.
    unlock(target);
    
    // Return the populated list of directory entries.
    return dir_content;
}