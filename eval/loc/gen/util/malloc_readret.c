#include "util.h"

/**
 * @brief Allocates and initializes a new struct read_ret instance.
 *
 * This function handles the memory allocation for a `struct read_ret` object.
 * It ensures that the newly created structure is properly initialized according
 * to the specification.
 *
 *
 * @return On success, a pointer to a newly allocated `struct read_ret` object
 *         where the `buf` field is initialized to NULL and the `num` field is
 *         initialized to 0.
 * @return On memory allocation failure, returns NULL.
 */
struct read_ret* malloc_readret() {
    struct read_ret* ptr = (struct read_ret*)malloc(sizeof(struct read_ret));

    if (ptr == NULL) {
        return NULL;
    }

    ptr->buf = NULL;
    ptr->num = 0;

    return ptr;
}