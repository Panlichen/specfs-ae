#include "util.h"

char** calculate(char* srcpath[], char* dstpath[])
{
    int count = 0;
    
    // Count common prefix components
    while (srcpath[count] && dstpath[count] && 
           strcmp(srcpath[count], dstpath[count]) == 0) {
        count++;
    }
    
    // Allocate result array with NULL terminator
    char** compath = malloc_path(count + 1);
    
    // Copy common components
    for (int i = 0; i < count; i++) {
        compath[i] = malloc_string(srcpath[i]);
    }
    
    // NULL terminate the array
    compath[count] = NULL;
    
    return compath;
}