#include "util.h"

void free_dirs(char *dirname[])
{
    if (dirname == NULL) {
        return;
    }
    
    for (size_t i = 0; dirname[i] != NULL; i++) {
        free(dirname[i]);
    }
}