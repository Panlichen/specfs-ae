#include "util.h"

char** malloc_dir_content(unsigned size) {
    return malloc(size * sizeof(char*));
}