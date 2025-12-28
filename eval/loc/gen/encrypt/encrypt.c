#include "encrypt.h"
#include <stdint.h>  // For uint32_t, essential for bitwise operations
#include <stdlib.h>  // For malloc() and free()
#include <string.h>  // For memcpy()

// Define constants for the XTEA algorithm
#define NUM_ROUNDS 32
#define DELTA 0x9E3779B9

/**
 * @brief Enciphers a single 64-bit block using XTEA. (Helper Function)
 * @param v A pointer to a 2-element array of uint32_t (64 bits) to be enciphered.
 * @param k A pointer to a 4-element array of uint32_t (128-bit key).
 */
static void xtea_encipher(uint32_t v[2], const uint32_t k[4]) {
    uint32_t v0 = v[0];
    uint32_t v1 = v[1];
    uint32_t sum = 0;
    
    for (unsigned int i = 0; i < NUM_ROUNDS; i++) {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
        sum += DELTA;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3]);
    }
    
    v[0] = v0;
    v[1] = v1;
}

/**
 * @brief Deciphers a single 64-bit block using XTEA. (Helper Function)
 * @param v A pointer to a 2-element array of uint32_t (64 bits) to be deciphered.
 * @param k A pointer to a 4-element array of uint32_t (128-bit key).
 */
static void xtea_decipher(uint32_t v[2], const uint32_t k[4]) {
    uint32_t v0 = v[0];
    uint32_t v1 = v[1];
    uint32_t sum = DELTA * NUM_ROUNDS;
    
    for (unsigned int i = 0; i < NUM_ROUNDS; i++) {
        v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3]);
        sum -= DELTA;
        v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
    }
    
    v[0] = v0;
    v[1] = v1;
}


void* encrypt_block(void *ptr, const unsigned char *key) {
    // 1. Allocate a new memory region for the encrypted data.
    void *encrypted_data = malloc(PAGE_SIZE);
    if (!encrypted_data) {
        return NULL; // Memory allocation failed
    }

    // 2. Copy original data to the new buffer to encrypt it there.
    memcpy(encrypted_data, ptr, PAGE_SIZE);

    // 3. Cast pointers for word-level access required by XTEA.
    uint32_t *data_words = (uint32_t *)encrypted_data;
    const uint32_t *key_words = (const uint32_t *)key;

    // 4. Iterate through the data in 64-bit (8-byte) chunks and encrypt each one.
    // A 4096-byte page contains 512 such chunks (4096 / 8 = 512).
    for (size_t i = 0; i < (PAGE_SIZE / sizeof(uint32_t) / 2); i++) {
        xtea_encipher(&data_words[i * 2], key_words);
    }

    return encrypted_data;
}

void* decrypt_block(void *ptr, const unsigned char *key) {
    // 1. Allocate new memory for the decrypted data.
    void *decrypted_data = malloc(PAGE_SIZE);
    if (!decrypted_data) {
        return NULL; // Memory allocation failed
    }

    // 2. Copy encrypted data to the new buffer to decrypt it there.
    memcpy(decrypted_data, ptr, PAGE_SIZE);

    // 3. Cast pointers for word-level access.
    uint32_t *data_words = (uint32_t *)decrypted_data;
    const uint32_t *key_words = (const uint32_t *)key;

    // 4. Iterate through the data in 64-bit chunks and decrypt each one.
    for (size_t i = 0; i < (PAGE_SIZE / sizeof(uint32_t) / 2); i++) {
        xtea_decipher(&data_words[i * 2], key_words);
    }
    
    return decrypted_data;
}

void free_block(void *ptr) {
    // Simply free the memory pointed to by ptr.
    // free(NULL) is safe and does nothing.
    free(ptr);
}