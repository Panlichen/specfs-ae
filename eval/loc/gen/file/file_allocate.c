#include "file.h"

/**
 * @brief Allocates file pages for a given offset and length.
 *
 * This function ensures that all necessary memory pages for a file, represented by an inode,
 * are allocated for a specified range. It calculates the start and end pages
 * corresponding to the given offset and length, and then iterates through this
 * range. If a page within the range has not yet been allocated (i.e., its entry in
 * the index table is NULL), a new page is allocated using `malloc_page()`.
 *
 * @param node Pointer to the inode structure representing the file.
 *             It is assumed that `node->file` is a valid pointer to an `indextb`.
 * @param offset The starting offset in the file from where to begin allocation.
 * @param len The number of bytes to allocate, starting from `offset`.
 *
 */
void file_allocate(struct inode *node, unsigned offset, unsigned len) {
    // Calculate the index of the first page affected by the allocation range.
    // Integer division automatically finds the page containing the 'offset' byte.
    unsigned start_page = offset / PG_SIZE;

    // Calculate the index of the last page affected by the allocation range.
    // The `(offset + len - 1)` expression finds the byte address of the last byte
    // in the range. Subtracting 1 is crucial to handle cases where the range ends
    // exactly on a page boundary. For example, if offset=0 and len=4096, the last byte is at 4095,
    // which is still in page 0. (0 + 4096 - 1) / 4096 = 0.
    unsigned end_page = (offset + len - 1) / PG_SIZE;

    // Iterate through all pages from the start to the end page (inclusive).
    for (unsigned i = start_page; i <= end_page; i++) {
        // Check if a page has already been allocated at this index.
        if (node->file->index[i] == NULL) {
            // If not, allocate a new page using the provided `malloc_page` function
            // and store its pointer in the index table.
            node->file->index[i] = malloc_page();
        }
    }
}