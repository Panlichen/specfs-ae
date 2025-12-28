#include "util.h"

/**
 * @brief Allocates a new, zero-initialized page of memory.
 *
 * @return A pointer to the newly allocated page.
 * @retval NULL If the memory allocation fails.
 */
unsigned char* malloc_page() {
    return calloc(1, PG_SIZE);
}