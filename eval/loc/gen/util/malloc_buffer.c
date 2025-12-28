#include "util.h"

/**
 * @brief Allocates a buffer of a specified length.
 *
 * This function acts as a wrapper around the standard `malloc` call to allocate
 * a memory buffer of a given size.
 *
 * @param len The length of the buffer to allocate in bytes.
 * @return A pointer to the newly allocated buffer.
 */
char* malloc_buffer(unsigned len) {
    return malloc(len);
}