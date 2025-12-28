#include "util.h"

/**
 * @brief Allocates memory for a new entry structure.
 *
 *
 * @return On successful allocation, a valid pointer to a newly allocated
 *         `struct entry`. The content of the allocated memory is uninitialized,
 *         as per the behavior of `malloc`. This fulfills **Post-Condition Case 1**.
 * @return On allocation failure, `NULL`. This fulfills **Post-Condition Case 2**.
 */
struct entry *malloc_entry() {
    return malloc(sizeof(struct entry));
}