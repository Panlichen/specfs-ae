#include "util.h"

char* malloc_string(const char* name) {
    size_t len = 0;
    while (name[len] != '\0') {
        len++;
    }
    
    char* copy = (char*)malloc(len + 1);
    
    for (size_t i = 0; i < len; i++) {
        copy[i] = name[i];
    }
    copy[len] = '\0';
    
    return copy;
}