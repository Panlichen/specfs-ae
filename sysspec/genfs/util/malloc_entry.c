#include "util.h"

struct entry *malloc_entry() {
    struct entry *new_entry = (struct entry *)malloc(sizeof(struct entry));
    if (new_entry == NULL) {
        return NULL;
    }
    return new_entry;
}