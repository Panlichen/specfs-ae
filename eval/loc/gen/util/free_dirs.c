#include "util.h"

/**
 * @brief Deallocates memory for each string in a NULL-terminated array of strings.
 *
 * This function implements the system algorithm to free a NULL-terminated array
 * of dynamically allocated strings. It iterates through the array and calls `free()`
 * on each non-NULL element.
 *
 * @param dirname A pointer to a NULL-terminated array of dynamically allocated
 *                strings.
 */
void free_dirs(char *dirname[]) {
    if (dirname == NULL) {
        return;
    }
    
    for (int i = 0; dirname[i] != NULL; i++) {
        free(dirname[i]);
    }
}