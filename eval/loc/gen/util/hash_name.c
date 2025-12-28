#include "util.h"

/**
 * @brief Computes a hash value for a given string using a polynomial rolling algorithm.
 * 
 * @param name A pointer to a null-terminated string to be hashed.
 *
 * @return The computed hash value. Returns `0` if `name` is `NULL` or an empty string.
 */
unsigned int hash_name(char* name)
{
    // Initialize the hash value to 0. This is the starting point for the algorithm.
    unsigned int hash = 0;
    
    if (name == NULL || name[0] == '\0') {
        return 0;
    }
    
    // Iterate through each character of the non-empty string.
    for (int i = 0; name[i] != '\0'; i++) {
        hash = (hash * 131) + (unsigned char)name[i];
    }
    
    // The final result is masked with 0x1ff.
    // This ensures the return value is constrained to the 9 least significant bits,
    // fitting within the required range of [0, 511].
    return hash & 0x1ff;
}