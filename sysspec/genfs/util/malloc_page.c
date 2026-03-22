#include "util.h"

unsigned char* malloc_page() {
    unsigned char* page = (unsigned char*)calloc(1, PG_SIZE);
    if (page == NULL) {
        return NULL;
    }
    return page;
}