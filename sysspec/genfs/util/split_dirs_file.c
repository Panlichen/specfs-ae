#include "util.h"

void split_dirs_file(const char *path, char *dirname[], char *filename) {
    if (path == NULL) return;
    
    // Step 1: Create a modifiable copy of the path
    char *path_copy = malloc_string(path);
    if (path_copy == NULL) return;
    
    char *saveptr = NULL;
    char *token = NULL;
    int idx = 0;
    
    // Step 2: Tokenize using strtok_r
    token = strtok_r(path_copy, "/", &saveptr);
    
    // Step 3: Populate directories
    while (token != NULL && idx < MAX_PATH_LEN - 1) {
        dirname[idx] = malloc_string(token);
        if (dirname[idx] == NULL) {
            // Cleanup and return on allocation failure
            for (int i = 0; i < idx; i++) {
                free(dirname[i]);
                dirname[i] = NULL;
            }
            free(path_copy);
            return;
        }
        idx++;
        token = strtok_r(NULL, "/", &saveptr);
    }
    
    // Step 4: Isolate filename
    if (idx > 0) {
        // Copy last token to filename
        strncpy(filename, dirname[idx-1], MAX_FILE_LEN - 1);
        filename[MAX_FILE_LEN - 1] = '\0';
        
        // Free and clear the last directory entry
        free(dirname[idx-1]);
        dirname[idx-1] = NULL;
    } else {
        // Path was empty or root only
        filename[0] = '\0';
    }
    
    // Step 5: Cleanup
    free(path_copy);
}