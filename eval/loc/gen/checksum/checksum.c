#include "checksum.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Standard CRC32 polynomial (Ethernet, ZIP, etc.)
#define CRC32_POLYNOMIAL 0xEDB88320L

// CRC32 lookup table and initialization flag
static uint32_t crc32_table[256];
static bool table_initialized = false;

/**
 * @brief Generates the CRC32 lookup table for faster calculations.
 *
 * This function is called automatically on the first checksum operation.
 */
static void crc32_generate_table(void) {
    if (table_initialized) {
        return;
    }
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (uint32_t j = 0; j < 8; j++) {
            crc = (crc & 1) ? (crc >> 1) ^ CRC32_POLYNOMIAL : (crc >> 1);
        }
        crc32_table[i] = crc;
    }
    table_initialized = true;
}

/**
 * @brief Updates a CRC32 value with a block of data.
 *
 * @param crc The initial CRC value.
 * @param buf A pointer to the data buffer.
 * @param size The size of the data buffer in bytes.
 * @return The updated CRC32 value.
 */
static uint32_t crc32_update(uint32_t crc, const void *buf, size_t size) {
    const uint8_t *p = (const uint8_t *)buf;
    while (size--) {
        crc = crc32_table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}

/**
 * @brief Calculates the CRC32 checksum for an inode's persistent data.
 *
 * This function traverses the persistent fields of the inode and its
 * associated data structures (directory and file index tables) to compute
 * a comprehensive checksum. Non-persistent data (e.g., mutexes, runtime
 * pointers) is explicitly ignored.
 *
 * @param node The inode for which to calculate the checksum.
 * @return The calculated 32-bit CRC value.
 */
static uint32_t checksum_calculate(const struct inode *node) {
    // Ensure the CRC32 lookup table is generated before first use.
    crc32_generate_table();

    uint32_t crc = 0xFFFFFFFF; // Initial CRC value

    // 1. Checksum primitive persistent metadata in the inode struct.
    crc = crc32_update(crc, &node->maj, sizeof(node->maj));
    crc = crc32_update(crc, &node->min, sizeof(node->min));
    crc = crc32_update(crc, &node->mode, sizeof(node->mode));
    crc = crc32_update(crc, &node->size, sizeof(node->size));

    // 2. If it's a directory, checksum its persistent entry data.
    if (node->dir != NULL) {
        for (int i = 0; i < DIRTB_NUM; i++) {
            // Traverse the linked list for this hash bucket
            for (entry *e = node->dir->tb[i]; e != NULL; e = e->next) {
                if (e->name) {
                    crc = crc32_update(crc, e->name, strlen(e->name));
                }
                // Checksum the inode number it points to
                crc = crc32_update(crc, &e->inum, sizeof(e->inum));
            }
        }
    }

    // 3. If it's a file, checksum its block pointers.
    if (node->file != NULL) {
        for (int i = 0; i < INDEXTB_NUM; i++) {
            if (node->file->index[i] != NULL) {
                // Checksum the block pointer value itself, not the data it points to.
                crc = crc32_update(crc, &node->file->index[i], sizeof(node->file->index[i]));
            }
        }
    }

    // Finalize the CRC value by inverting the bits.
    return crc ^ 0xFFFFFFFF;
}

bool checksum_validate(const struct inode *node) {
    if (node == NULL) {
        return false;
    }
    return node->checksum == checksum_calculate(node);
}

void checksum_update(struct inode *node) {
    if (node == NULL) {
        return;
    }
    node->checksum = checksum_calculate(node);
}