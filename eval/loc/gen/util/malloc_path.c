#include "util.h"

/**
 * @brief Allocates an array of string pointers (paths).
 *
 * @param len [Pre-Condition] A positive integer representing the number of string
 *            pointers (paths) to allocate.
 *
 * @return [Post-Condition] On success, a pointer to the newly allocated array of
 *         string pointers (char**), with each element initialized to NULL.
 * @return On memory allocation failure, returns NULL.
 */
char** malloc_path(unsigned len) {
    // Allocate memory for an array of 'len' pointers to characters.
    char** paths = malloc(sizeof(char*) * len);

    // Check if the memory allocation by malloc() was successful.
    if (paths == NULL) {
        return NULL;
    }

    // Iterate through the newly allocated array of pointers.
    for (unsigned i = 0; i < len; i++) {
        paths[i] = NULL;
    }

    return paths;
}