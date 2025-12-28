#include "util.h"

/**
 * @brief Calculates the number of non-NULL elements in a NULL-terminated array of strings.
 * 
 * This function implements the `getlen` operation as specified. It takes an array
 * of string pointers and counts how many non-NULL pointers it contains before
 * encountering the first NULL pointer, which marks the end of the array.
 * 
 * @param path A pointer to an array of character pointers (strings).
 *             [Pre-Condition] This array must be terminated by a NULL pointer.
 * 
 * @return An integer representing the number of non-NULL elements in `path`.
 *         [Post-Condition] This is the count of strings before the NULL terminator.
 */
int getlen(char* path[]) {
    int len = 0;
    
    if (path == NULL) {
        return 0;
    }
    
    while (path[len] != NULL) {
        len++;
    }

    return len;
}