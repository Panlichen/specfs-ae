#include "path.h"

/**
 * @brief Traverses a file system path starting from a given inode, while retaining the lock on the starting inode.
 *
 * This function handles the first step of a path traversal by finding the
 * first component in the directory `cur`. It then delegates the remainder of the
 * traversal to the standard `locate` function. Its primary distinction is that
 * it does not release the lock on the initial inode `cur`, which is a
 * requirement for certain higher-level operations (e.g., rename).
 *
 * @param cur A pointer to the starting directory inode. This inode MUST be locked
 *            by the caller before this function is invoked.
 * @param path A NULL-terminated array of strings representing the path components
 *             to traverse, relative to `cur`.
 *
 * @return On success, returns a pointer to the target inode, which will be locked.
 *         The lock on the initial inode `cur` is retained.
 * @return On failure (e.g., a path component is not found), returns NULL. The
 *         lock on `cur` is still retained.
 *
 */
struct inode* locate_hold(struct inode *cur, char *path[]) {
    // Per the specification, if the path is empty or NULL, the target is the
    // current inode itself. The caller is expected to already hold the lock on `cur`.
    if (path == NULL || path[0] == NULL) {
        return cur;
    }

    // Find the inode corresponding to the first component of the path within the
    // current directory `cur`.
    struct inode *next_inum = inode_find(cur, path[0]);

    // If the first component is not found, the traversal fails.
    // Return NULL, retaining the caller's lock on `cur` as required by the post-condition.
    if (next_inum == NULL) {
        return NULL;
    }

    // The next inode was found. Lock it immediately. This is a critical lock-coupling
    // step: by holding the lock on both the parent (`cur`) and the child (`next_inum`),
    // we prevent race conditions where the link between them could be broken by another process.
    lock(next_inum);

    // Check if the first component was also the final component of the path.
    if (path[1] == NULL) {
        // This is the target inode. According to the post-condition, we must
        // return it while holding locks on both `cur` and `next_inum`.
        return next_inum;
    }

    // The path has more components. Delegate the rest of the traversal to the
    // `locate` function, starting from the now-locked `next_inum` and passing
    // the remainder of the path.
    struct inode *result = locate(next_inum, &path[1]);

    // The `locate` function, by its own specification, will release the lock
    // on its starting node (`next_inum`) and all other intermediate nodes.
    // However, the lock on our original `cur` was never passed to `locate`,
    // so it remains held, fulfilling this function's main guarantee.
    // The `result` will either be the locked target inode or NULL.
    return result;
}