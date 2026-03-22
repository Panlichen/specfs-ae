#include "util.h"

unsigned int hash_name(char* name) {
    if (!name || !*name) {
        return 0;
    }
    
    unsigned int hash = 0;
    while (*name) {
        hash = hash * 131 + (unsigned char)*name;
        name++;
    }
    return hash & 0x1ff;
}