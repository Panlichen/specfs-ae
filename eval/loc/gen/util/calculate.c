#include "util.h"

/**
 * @brief A helper function to compare two strings for exact equality.
 *
 * @param a The first null-terminated string.
 * @param b The second null-terminated string.
 * @return 1 if the strings are identical, 0 otherwise.
 */
static int streq(const char* a, const char* b) {
    // Iterate through both strings simultaneously.
    while (*a && *b) {
        // If a character mismatch is found, the strings are not equal.
        if (*a != *b) return 0;
        a++;
        b++;
    }
    // The strings are equal only if both have reached their null terminators at the same time.
    return (*a == '\0' && *b == '\0');
}

/**
 * @brief Calculates the longest common prefix of two paths.
 *
 * @param srcpath A NULL-terminated array of strings representing the source path.
 * @param dstpath A NULL-terminated array of strings representing the destination path.
 * @return A new, NULL-terminated array of strings containing the longest common prefix.
 */
char** calculate(char* srcpath[], char* dstpath[]) {
    int i = 0;

    while (srcpath[i] && dstpath[i] && streq(srcpath[i], dstpath[i])) {
        // The variable `i` will hold the length of the common prefix.
        i++;
    }
    
    char** compath = malloc_path(i + 1);

    // Copy the common path components into the newly allocated array.
    for (int j = 0; j < i; j++) {
        // Use malloc_string to create a copy of each path component string.
        compath[j] = malloc_string(srcpath[j]);
    }

    compath[i] = NULL;
    
    return compath;
}