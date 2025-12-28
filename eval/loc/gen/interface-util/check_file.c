#include "interface-util.h"

/**
 * @brief Verifies that a given inode represents a file and not a directory.
 *
 * This function checks if the provided inode pointer is valid (not NULL) and if its
 * mode indicates it is not a directory. It also handles lock management based on the
 * outcome of the check, as specified by the refined requirements.
 *
 * @param inum A pointer to the inode structure to be validated.
 *
 *
 * @return 0 on success, indicating `inum` is a valid file. In this case, the
 *         lock for `inum` remains held by the caller.
 * @return 1 on failure, indicating `inum` is NULL or is a directory. If the
 *         failure is due to `inum` being a directory, its lock is released
 *         by this function.
 */
int check_file(struct inode *inum)
{
	// Per the specification, handle the pre-condition case where the inode
	// pointer itself is NULL.
	if (inum == NULL)
		// Return 1 (failure) as specified for a NULL inode (Case 2).
		return 1;

	// Check if the inode's mode is DIR_MODE.
	if (inum->mode == DIR_MODE) {
		// As per the refined specification, if the inode is a directory,
		// we must release its lock before returning.
		unlock(inum);
		// Return 1 (failure) because the inode represents a directory (Case 2).
		return 1;
	}

	// If the inode is not NULL and not a directory, the check succeeds (Case 1).
	// The lock remains held, fulfilling the post-condition for success.
	return 0;
}