#include "util.h"

/**
 * @brief Allocates memory for an array of character pointers.
 *
 * @param size The number of character pointers to allocate space for.
 *
 * @return A pointer to the newly allocated block of memory (`char**`) on success
 *         (Case 1: Successful Allocation).
 * @return `NULL` if the memory allocation fails.
 */
char** malloc_dir_content(unsigned size) {
    return (char**)malloc(size * sizeof(char*));
}