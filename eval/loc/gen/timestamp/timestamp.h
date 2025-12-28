#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <time.h>   // Required for struct timespec and clockid_t
#include "common.h" // Required for the definition of struct inode

// Defines the types of timestamps that can be accessed or modified.
#define ACCESS_TIME 0       // Last time the file was accessed
#define MODIFICATION_TIME 1 // Last time the file's content was modified
#define CHANGE_TIME 2       // Last time the file's metadata (inode) was changed

/**
 * @brief Sets the current high-resolution time to a specific timestamp field in an inode.
 * @param node A pointer to the inode structure to modify.
 * @param type The type of timestamp to set (ACCESS_TIME, MODIFICATION_TIME, or CHANGE_TIME).
 */
void set_current_time(struct inode* node, int type);

/**
 * @brief Retrieves a specific high-resolution timestamp from an inode.
 * @param node A pointer to the inode structure to read from.
 * @param type The type of timestamp to get (ACCESS_TIME, MODIFICATION_TIME, or CHANGE_TIME).
 * @return A timespec structure containing the reconstructed time, including nanoseconds.
 */
struct timespec get_current_time(struct inode* node, int type);

#endif // TIMESTAMP_H