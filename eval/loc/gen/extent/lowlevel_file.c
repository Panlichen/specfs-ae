#include "lowlevel_file.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


unsigned char* malloc_pages(unsigned num_pages) {
    if (num_pages == 0) {
        return NULL;
    }
    // calloc ensures the memory is zero-initialized, as required.
    return (unsigned char*) calloc(num_pages, PG_SIZE);
}


/**
 * @brief Finds the physical memory address for a logical file offset.
 *
 * Traverses the inode's extent list to find the physical page for the given
 * offset. It optimizes I/O by calculating and returning the number of
 * contiguous bytes available from that address within the same extent.
 *
 * @param node The file's inode.
 * @param offset The logical byte offset within the file.
 * @param contig_len_out Output pointer for the number of contiguous bytes.
 * @return Physical memory address, or NULL if the offset is in an unallocated region.
 */
static unsigned char* find_phys_addr(struct inode *node, unsigned offset, unsigned *contig_len_out) {
    unsigned target_page_idx = offset / PG_SIZE;
    unsigned offset_in_page = offset % PG_SIZE;

    unsigned current_page_idx = 0;
    struct extent *ext = node->ext_head;

    while (ext != NULL) {
        // Check if the target logical page is within the current extent
        if (target_page_idx < current_page_idx + ext->len_pages) {
            unsigned page_idx_in_extent = target_page_idx - current_page_idx;
            unsigned char *phys_addr = ext->start_page + (page_idx_in_extent * PG_SIZE) + offset_in_page;
            *contig_len_out = (ext->len_pages - page_idx_in_extent) * PG_SIZE - offset_in_page;
            return phys_addr;
        }
        current_page_idx += ext->len_pages;
        ext = ext->next;
    }

    // Offset is in a hole (unallocated region)
    *contig_len_out = (unsigned)-1; // Signal a large block of zeros
    return NULL;
}


void file_allocate(struct inode *node, unsigned offset, unsigned len) {
    if (len == 0) return;

    unsigned required_end_offset = offset + len;
    if (required_end_offset == 0) return; // Avoid underflow
    unsigned required_total_pages = (required_end_offset - 1) / PG_SIZE + 1;

    // Find the last extent and count current total pages
    unsigned current_total_pages = 0;
    struct extent *last_ext = NULL;
    if (node->ext_head) {
        last_ext = node->ext_head;
        while (1) {
            current_total_pages += last_ext->len_pages;
            if (last_ext->next == NULL) break;
            last_ext = last_ext->next;
        }
    }

    if (required_total_pages <= current_total_pages) return; // Already allocated

    unsigned pages_to_alloc = required_total_pages - current_total_pages;
    unsigned char *new_block = malloc_pages(pages_to_alloc);
    if (!new_block) {
        perror("malloc_pages failed in file_allocate");
        return;
    }

    // Create and append a new extent
    struct extent *new_ext = (struct extent*) malloc(sizeof(struct extent));
    if (!new_ext) {
        free(new_block);
        perror("malloc failed for new extent");
        return;
    }
    new_ext->start_page = new_block;
    new_ext->len_pages = pages_to_alloc;
    new_ext->next = NULL;

    if (last_ext == NULL) {
        node->ext_head = new_ext;
    } else {
        last_ext->next = new_ext;
    }
}

void file_read(struct inode *node, unsigned offset, unsigned len, char *data) {
    if (!node || !data || len == 0) return;

    unsigned bytes_read = 0;
    while (bytes_read < len) {
        unsigned current_offset = offset + bytes_read;
        if (current_offset >= node->size) break; // Don't read past EOF

        unsigned remaining_len = len - bytes_read;
        
        unsigned contig_len = 0;
        unsigned char *phys_addr = find_phys_addr(node, current_offset, &contig_len);

        unsigned bytes_to_copy = remaining_len < contig_len ? remaining_len : contig_len;
        if (current_offset + bytes_to_copy > node->size) {
            bytes_to_copy = node->size - current_offset;
        }

        if (phys_addr) {
            memcpy(data + bytes_read, phys_addr, bytes_to_copy);
        } else {
            memset(data + bytes_read, 0, bytes_to_copy); // Read from a hole
        }
        bytes_read += bytes_to_copy;
    }

    // Fill the remainder of the buffer with zeros if read was partial (past EOF)
    if (bytes_read < len) {
        memset(data + bytes_read, 0, len - bytes_read);
    }
}

void file_write(struct inode *node, unsigned offset, unsigned len, const char *data) {
    if (!node || len == 0) return;

    file_allocate(node, offset, len);

    if (offset + len > node->size) {
        node->size = offset + len;
    }

    unsigned bytes_written = 0;
    while (bytes_written < len) {
        unsigned current_offset = offset + bytes_written;
        unsigned remaining_len = len - bytes_written;

        unsigned contig_len = 0;
        unsigned char *phys_addr = find_phys_addr(node, current_offset, &contig_len);

        if (!phys_addr) {
            fprintf(stderr, "Error: physical address not found for write at offset %u after allocation.\n", current_offset);
            break;
        }

        unsigned bytes_to_copy = remaining_len < contig_len ? remaining_len : contig_len;
        
        if (data) {
            memcpy(phys_addr, data + bytes_written, bytes_to_copy);
        } else {
            memset(phys_addr, 0, bytes_to_copy); // For clear_file
        }
        bytes_written += bytes_to_copy;
    }
}

void clear_file(struct inode *node, unsigned start, unsigned len) {
    file_write(node, start, len, NULL);
}

void free_all_extents(struct inode *node) {
    struct extent *current = node->ext_head;
    while (current != NULL) {
        struct extent *next = current->next;
        free(current->start_page); // Free the block of pages
        free(current);             // Free the extent structure itself
        current = next;
    }
    node->ext_head = NULL;
}