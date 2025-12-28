#ifndef ENCRYPT_H
#define ENCRYPT_H

// Define the size of a page/block to be encrypted/decrypted, as per the specification.
#define PAGE_SIZE 4096

/**
 * @brief Encrypts a block of data using the XTEA algorithm.
 * * Allocates a new memory region for the encrypted data. The caller is responsible 
 * for freeing this memory using free_block().
 *
 * @param ptr A pointer to the PAGE_SIZE memory region containing the plaintext data.
 * @param key A pointer to a 16-byte (128-bit) encryption key.
 * @return A pointer to the newly allocated ciphertext block.
 * Returns NULL if memory allocation fails.
 */
void* encrypt_block(void *ptr, const unsigned char *key);

/**
 * @brief Decrypts a block of data using the XTEA algorithm.
 * * Allocates a new memory region for the decrypted data. The caller is responsible 
 * for freeing this memory using free_block().
 *
 * @param ptr A pointer to the PAGE_SIZE memory region containing the ciphertext.
 * @param key A pointer to a 16-byte (128-bit) decryption key.
 * @return A pointer to the newly allocated plaintext block.
 * Returns NULL if memory allocation fails.
 */
void* decrypt_block(void *ptr, const unsigned char *key);

/**
 * @brief Frees a memory block allocated by encrypt_block or decrypt_block.
 *
 * @param ptr A pointer to the memory region to be freed.
 */
void free_block(void *ptr);

#endif // ENCRYPT_H