#include "util.h"

/**
 * @brief Deallocates memory for a NULL-terminated array of strings.
 * @details This function first iterates through the array, freeing each individual
 *          string. After all strings have been deallocated, it frees the
 *          memory for the array of pointers itself.
 * 
 * @param path A pointer to a NULL-terminated array of strings. Each string
 *             and the array itself are expected to be dynamically allocated.
 *
 */
void free_path(char** path) {
    if (!path) {
        return;
    }
    
    for (int i = 0; path[i] != NULL; i++) {
        free(path[i]);
    }
    
    free(path);
}