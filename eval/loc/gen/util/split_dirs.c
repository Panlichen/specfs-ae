#include "util.h"

/**
 * @brief Splits a file path string into its constituent directory/file components.
 *
 *
 * @param path      [in] A pointer to a valid, NULL-terminated string representing the
 *                  file path to be split (e.g., "/dir1/dir2/file").
 * @param dirname   [out] A pointer to an array of `char*` which will be populated
 *                  with pointers to newly allocated strings. Each string will
 *                  contain one component of the original path.
 * 
 */
void split_dirs(const char *path, char *dirname[]) {
    char *path_copy = malloc_string(path);
    
    char *saveptr = NULL;
    char *token = strtok_r(path_copy, "/", &saveptr);
    int idx = 0; // Index to keep track of the current position in `dirname`.
    
    // Loop until all tokens have been extracted from the path copy.
    while (token != NULL) {
        if (strlen(token) >= MAX_FILE_LEN) {
            // Free the temporary path copy to prevent a memory leak.
            free(path_copy);
            // Ensure the rest of the dirname array is NULL-terminated for the caller.
            while (idx < MAX_PATH_LEN) {
                dirname[idx++] = NULL;
            }
            return;
        }
        
        dirname[idx] = malloc_string(token);
        idx++;
        
        if (idx >= MAX_PATH_LEN) {
            // Free the temporary path copy.
            free(path_copy);
            // Ensure the next pointer is NULL if there is space.
            if (idx < MAX_PATH_LEN) {
                dirname[idx] = NULL;
            }
            return;
        }
        
        token = strtok_r(NULL, "/", &saveptr);
    }
    
    dirname[idx] = NULL;
    
    free(path_copy);
}