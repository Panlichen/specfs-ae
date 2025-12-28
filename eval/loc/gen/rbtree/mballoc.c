#include "mballoc.h"
#include <stdlib.h> // For malloc() and free()
#include <stdio.h>  // For NULL

#include "rbtree.h" // Assumed to contain the rbtree function prototypes

/*
 * This function is defined externally, as per the specification.
 * It allocates a specified number of contiguous pages.
 */
extern unsigned char* malloc_contigous_pages(unsigned num);

/* Function Prototypes for static helpers */
static void create_extent(struct inode *node, unsigned start_page, unsigned length, unsigned char *data);
static unsigned try_prealloc(struct inode *node, unsigned offset, unsigned len);

/**
 * @brief Creates a new extent and appends it to the inode's extent list.
 *
 * (This function is unchanged from the original implementation.)
 */
static void create_extent(struct inode *node, unsigned start_page, unsigned length, unsigned char *data) {
    Extent *new_extent = (Extent *)malloc(sizeof(Extent));
    if (!new_extent) {
        return; // Error handling would be more robust in a real system
    }
    new_extent->start_page = start_page;
    new_extent->length = length;
    new_extent->data = data;
    new_extent->next = NULL;

    if (node->extents == NULL) {
        node->extents = new_extent;
    } else {
        Extent *current = node->extents;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_extent;
    }
}

/**
 * @brief Tries to satisfy an allocation request from the inode's preallocated blocks using a red-black tree.
 *
 * This function searches the red-black tree for a preallocated chunk that can
 * satisfy the request. It looks for a chunk whose free space is contiguous
 * with the requested offset. If found, an extent is created, the prealloc node
 * is updated, and if fully consumed, it is removed from the tree and freed.
 *
 * @param node The inode containing the prealloc tree.
 * @param offset The logical starting offset for the allocation request.
 * @param len The total number of blocks requested.
 * @return The number of blocks that still need to be allocated.
 */
static unsigned try_prealloc(struct inode *node, unsigned offset, unsigned len) {
    if (len == 0 || node == NULL || node->prealloc_tree == NULL) {
        return len;
    }

    // 🌳 Find the prealloc node with the greatest start offset that is <= the target offset.
    struct rb_node *candidate_node = rb_find_max_lte(node->prealloc_tree, offset);

    if (candidate_node == NULL) {
        return len; // No suitable preallocation candidate found.
    }

    Prealloc *pa = candidate_node->prealloc;

    // The free space in a prealloc block is always at the end.
    // Calculate the logical starting block of that free space.
    unsigned free_start_lblock = pa->pa_lstart + (pa->pa_len - pa->pa_free);

    // Check if the allocation request starts exactly where the free space begins.
    // This ensures contiguous allocation from the preallocated chunk.
    if (offset == free_start_lblock && pa->pa_free > 0) {
        // We have a match! Allocate from this prealloc node.
        unsigned alloc_count = (len < pa->pa_free) ? len : pa->pa_free;

        // Calculate the physical data pointer for this new allocation.
        unsigned long used_blocks = pa->pa_len - pa->pa_free;
        unsigned char *data_ptr = pa->pa_data + (used_blocks * PAGE_SIZE);

        create_extent(node, offset, alloc_count, data_ptr);

        // Update the prealloc block's state.
        pa->pa_free -= alloc_count;

        // If the prealloc block is now fully used, remove it from the tree and free its memory.
        if (pa->pa_free == 0) {
            rb_delete(node, candidate_node); // Unlink the node from the tree.
            
            // The data buffer `pa->pa_data` is now managed by the new extent.
            // We free the Prealloc struct and its rb_node container.
            // NOTE: This assumes `rb_delete` only unlinks the node, and the
            // caller is responsible for freeing the associated memory.
            free(pa);
            free(candidate_node);
        }
        return len - alloc_count; // Return remaining length
    }

    // The found candidate was not suitable (e.g., offset is in a hole or doesn't align).
    return len;
}

/**
 * @brief Allocates 'len' blocks starting at 'offset' for the given inode.
 *
 * This is the main allocation function, updated to use the red-black tree.
 */
void mballoc(struct inode *node, unsigned offset, unsigned len) {
    if (len == 0 || node == NULL) {
        return;
    }

    // Step 1: Attempt to fulfill the allocation from the prealloc tree.
    unsigned remaining_len = try_prealloc(node, offset, len);

    if (remaining_len == 0) {
        return; // Success! The request was fully satisfied from preallocations.
    }

    // Step 2: If blocks remain, fall back to a new, larger allocation.
    unsigned new_alloc_offset = offset + (len - remaining_len);
    unsigned normalized_len = ((remaining_len + ALLOC_GRANULARITY - 1) / ALLOC_GRANULARITY) * ALLOC_GRANULARITY;
    
    unsigned char *data_ptr = malloc_contigous_pages(normalized_len);
    if (data_ptr == NULL) {
        return; // Allocation failed.
    }

    // Create an extent for the portion we actually needed for this call.
    create_extent(node, new_alloc_offset, remaining_len, data_ptr);

    // Step 3: If we allocated extra space, add it to the prealloc tree for future use.
    unsigned extra_len = normalized_len - remaining_len;
    if (extra_len > 0) {
        Prealloc *new_pa = (Prealloc *)malloc(sizeof(Prealloc));
        if (new_pa == NULL) {
            // Failed to create prealloc struct. The extra memory is now leaked.
            return;
        }

        new_pa->pa_lstart = new_alloc_offset;
        new_pa->pa_data = data_ptr;
        new_pa->pa_len = normalized_len;
        new_pa->pa_free = extra_len;

        // Insert the new prealloc block into the red-black tree.
        rb_insert(node, new_pa);
    }
}