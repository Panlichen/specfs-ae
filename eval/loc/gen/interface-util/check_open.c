#include "interface-util.h"

/**
 * @brief Checks if an inode can be opened with a given mode and releases its lock.
 *
 *
 * @param inum A pointer to the inode structure to check. Can be NULL.
 * @param mode The desired file mode for opening (e.g., DIR_MODE).
 *
 * @return 0 on success, indicating the modes are compatible.
 * @return 1 on failure, indicating either a NULL inode was provided or the 
 *         modes are incompatible (e.g., trying to open a directory as a file).
 */
int check_open(struct inode *inum, unsigned mode) {
    // Handle NULL inode case as per Specification Case 2a.
    // If the inode pointer is NULL, it's an immediate failure.
    if (inum == NULL) {
        return 1;
    }
    
    // Per the pre-condition, the lock for 'inum' is owned upon entry.
    
    // Check for mode compatibility. This variable will be true if modes are compatible.
    int compatible;

    // First, check if the inode represents a directory.
    if (inum->mode == DIR_MODE) {
        // If the inode is a directory, the requested mode must also be DIR_MODE.
        // This covers Specification Case 1 (success) and Case 2b (failure).
        compatible = (mode == DIR_MODE);
    } else {
        // If the inode is not a directory, the requested mode must also not be DIR_MODE.
        // This covers Specification Case 1 (success) and Case 2c (failure).
        compatible = (mode != DIR_MODE);
    }
    
    // Fulfill the post-condition: the lock must be released before returning.
    unlock(inum);
    
    // Convert the boolean 'compatible' result to the specified return value.
    // If compatible is true (1), return 0 for success.
    // If compatible is false (0), return 1 for failure.
    return compatible ? 0 : 1;
}