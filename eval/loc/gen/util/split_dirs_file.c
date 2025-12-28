#include "util.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief Splits an absolute file path into its directory components and a filename.
 *
 *
 * @param path A non-NULL, null-terminated string representing the absolute file path.
 * @param dirname An array of `char*` pointers (size >= MAX_PATH_LEN), initialized to NULLs.
 *                On successful return, it will be populated with newly allocated strings
 *                for each directory component. The caller is responsible for freeing
 *                these strings.
 * @param filename A character buffer (size >= MAX_FILE_LEN) to store the filename.
 */
void split_dirs_file(const char *path, char *dirname[], char *filename) {
    // If the path is invalid, empty, or just the root directory, there's nothing to split.
    if (path == NULL || path[0] == '\0' || 
        (path[0] == '/' && path[1] == '\0')) {
        return;
    }

    char *path_copy = malloc_string(path);
    if (path_copy == NULL) {
        return;  // Abort if memory allocation fails.
    }

    // Variables for tokenization using the re-entrant strtok_r function.
    char *saveptr = NULL; // `saveptr` is used by strtok_r to maintain state between calls.
    char *token = strtok_r(path_copy, "/", &saveptr);
    int dir_index = 0;    // Index for the dirname array.

    while (token != NULL && dir_index < MAX_PATH_LEN - 1) {
        char *dir_component = malloc_string(token);
        if (dir_component == NULL) {
            free(path_copy);
            return;  // Abort if memory allocation fails.
        }
        
        // Store the pointer to the newly allocated directory string in the array.
        dirname[dir_index] = dir_component;
        dir_index++;
        
        // Get the next token from the path string.
        token = strtok_r(NULL, "/", &saveptr);
    }

    if (dir_index > 0) {
        // The index of the last token, which is treated as the filename.
        int last_index = dir_index - 1;
        
        // Copy the string content of the last token into the filename buffer.
        if (dirname[last_index] != NULL) {
            // Use strncpy for safe copying to a fixed-size buffer.
            strncpy(filename, dirname[last_index], MAX_FILE_LEN - 1);
            // Ensure the filename is always null-terminated.
            filename[MAX_FILE_LEN - 1] = '\0';
        }
        
        // The last token was the filename, so it should not be in the dirname array.
        // Free the memory that was allocated for it.
        free(dirname[last_index]);
        // Set the pointer in the array to NULL. This fulfills the post-condition:
        dirname[last_index] = NULL;
        
        if (last_index + 1 < MAX_PATH_LEN) {
            dirname[last_index + 1] = NULL;
        }
    } else if (token == NULL && dir_index == 0) {
        // This condition checks if the tokenization loop did not run at all.
        // It manually re-parses the original path to find the filename.
        if (path[0] == '/' && path[1] != '\0') {
            const char *name_start = path + 1;
            strncpy(filename, name_start, MAX_FILE_LEN - 1);
            filename[MAX_FILE_LEN - 1] = '\0'; // Ensure null termination.
        }
    }

    free(path_copy);
}