#include "util.h"

/**
 * @brief Frees the memory associated with an entry.
 *
 * This function deallocates the memory for an entry's name and the entry
 * structure itself. It assumes both were dynamically allocated.
 *
 * @param en A pointer to the entry structure to be freed.
 *
 */
void free_entry(struct entry* en) {
    free(en->name);
    free(en);
}