#include "util.h"

/**
 * @brief Dynamically allocates a new string and copies the content of a source string into it.
 *
 *
 * @param name [SPECIFICATION] Pre-Condition: A non-NULL pointer to the source
 *             string to be copied.
 *
 * @return [SPECIFICATION] Post-Condition: A pointer to the newly allocated string
 *         containing a copy of `name`. The specification assumes `malloc` succeeds,
 *         so this function will not return NULL.
 */
char* malloc_string(const char* name) {
    size_t len = strlen(name) + 1;

    char* copy = (char*)malloc(len);
    memcpy(copy, name, len);

    return copy;
}