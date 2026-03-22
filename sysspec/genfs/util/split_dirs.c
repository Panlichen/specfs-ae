#include "util.h"

void split_dirs(const char *path, char *dirname[]) {
    // Create a modifiable copy of the path
    char *path_copy = malloc_string(path);
    
    // Tokenize the copy using strtok_r
    char *saveptr;
    char *token = strtok_r(path_copy, "/", &saveptr);
    int i = 0;
    
    while (token != NULL) {
        // Assert bounds as specified
        assert(i < MAX_PATH_LEN);
        assert(strlen(token) <= MAX_FILE_LEN);
        
        // Allocate and store the token
        dirname[i] = malloc_string(token);
        
        // Get next token
        token = strtok_r(NULL, "/", &saveptr);
        i++;
    }
    
    // Free the temporary copy
    free(path_copy);
}