#ifndef CHECKSUM_H_
#define CHECKSUM_H_

#include "common.h"
#include <stdbool.h>

/**
 * @brief Validates the checksum of an inode's metadata.
 *
 * Calculates the CRC32 checksum of the inode's persistent metadata and
 * compares it with the value stored in 'node->checksum'.
 *
 * @param node A constant pointer to the inode to validate. Must not be NULL.
 * @return Returns true if the calculated checksum matches the stored one,
 * false otherwise.
 */
bool checksum_validate(const struct inode *node);

/**
 * @brief Calculates and updates the checksum of an inode's metadata.
 *
 * Calculates the CRC32 checksum of the inode's persistent metadata and
 * stores the result in the 'node->checksum' field.
 *
 * @param node A pointer to the inode to update. Must not be NULL.
 */
void checksum_update(struct inode *node);

#endif // CHECKSUM_H_