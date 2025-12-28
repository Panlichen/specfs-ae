#include "mballoc.h"
#include <stdlib.h> // For malloc() and free()
#include <stdio.h>  // For NULL

/*
 * This function is defined externally, as per the specification.
 * It allocates a specified number of contiguous pages.
 */
extern unsigned char* malloc_contigous_pages(unsigned num);


static void create_extent(struct inode *node, unsigned start_page, unsigned length, unsigned char *data);
static unsigned try_prealloc(struct inode *node, unsigned offset, unsigned len);


/**
 * @brief Creates a new extent and appends it to the inode's extent list.
 *
 * An extent represents a contiguous range of allocated blocks in a file.
 * This function allocates a new Extent structure, initializes it with the
 * provided details, and adds it to the end of the inode's linked list of extents.
 *
 * @param node The inode to which the extent will be added.
 * @param start_page The logical starting page of the extent.
 * @param length The number of contiguous pages in the extent.
 * @param data A pointer to the memory where the data for these pages is stored.
 */
static void create_extent(struct inode *node, unsigned start_page, unsigned length, unsigned char *data) {
    // Allocate memory for the new extent structure
    Extent *new_extent = (Extent *)malloc(sizeof(Extent));
    if (!new_extent) {
        // In a real system, this would trigger more robust error handling.
        return;
    }

    // Initialize the new extent's fields
    new_extent->start_page = start_page;
    new_extent->length = length;
    new_extent->data = data;
    new_extent->next = NULL;

    // Append the new extent to the inode's list
    if (node->extents == NULL) {
        // The list is empty; this becomes the first extent.
        node->extents = new_extent;
    } else {
        // Traverse to the end of the list to append.
        Extent *current = node->extents;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_extent;
    }
}

/**
 * @brief Tries to satisfy an allocation request from the inode's preallocated blocks.
 *
 * This function iterates through the list of preallocated memory chunks associated
 * with the inode. If a preallocated chunk is found that can contiguously satisfy
 * the beginning of the current allocation request, it is used. An extent is created
 * for the fulfilled portion. If a preallocated block is fully consumed, it is freed.
 *
 * @param node The inode containing the prealloc list.
 * @param offset The logical starting offset for the allocation request.
 * @param len The total number of blocks requested.
 * @return The number of blocks that still need to be allocated after this attempt.
 */
static unsigned try_prealloc(struct inode *node, unsigned offset, unsigned len) {
    unsigned remaining_len = len;
    unsigned current_offset = offset;
    // Use a pointer-to-pointer to simplify removing items from the linked list.
    Prealloc **p_pa = &node->preallocs;

    while (*p_pa != NULL && remaining_len > 0) {
        Prealloc *pa = *p_pa;

        // The free space in a prealloc block is always at the end.
        // Calculate the logical start of that free space.
        unsigned free_start_lblock = pa->pa_lstart + (pa->pa_len - pa->pa_free);

        // Check if the current allocation request starts exactly where the free space begins.
        // This ensures contiguous allocation.
        if (current_offset == free_start_lblock && pa->pa_free > 0) {
            // Determine how many blocks we can take from this prealloc chunk.
            unsigned alloc_count = (remaining_len < pa->pa_free) ? remaining_len : pa->pa_free;

            // Calculate the physical data pointer for this new allocation.
            unsigned char *data_ptr = pa->pa_data + ((pa->pa_len - pa->pa_free) * PAGE_SIZE);

            // Create an extent for the newly allocated space.
            create_extent(node, current_offset, alloc_count, data_ptr);

            // Update the prealloc block's state.
            pa->pa_free -= alloc_count;

            // Update the overall allocation request state.
            remaining_len -= alloc_count;
            current_offset += alloc_count;

            // If the prealloc block is now fully used, remove it from the list.
            if (pa->pa_free == 0) {
                *p_pa = pa->next; // Unlink from the list.
                // The data buffer `pa->pa_data` is now managed by one or more extents.
                // We only free the Prealloc structure itself.
                free(pa);
                // *p_pa is now the next item, so we don't advance the p_pa pointer.
                continue;
            }
        }
        // Move to the next prealloc block in the list.
        p_pa = &((*p_pa)->next);
    }

    return remaining_len;
}

/**
 * @brief Allocates 'len' blocks starting at 'offset' for the given inode.
 *
 * This function handles block allocation for an inode. It first attempts to
 * satisfy the request from pre-allocated blocks. If the pre-allocated space
 * is insufficient, it falls back to allocating a new contiguous memory region.
 * The size of this new region is rounded up to the nearest multiple of
 * ALLOC_GRANULARITY. Any extra space from this new allocation is saved as a
 * new pre-allocated block for future use.
 *
 * @param node The inode for which to allocate blocks.
 * @param offset The logical starting block (page) offset within the file.
 * @param len The number of blocks (pages) to allocate.
 */
void mballoc(struct inode *node, unsigned offset, unsigned len) {
    if (len == 0 || node == NULL) {
        return;
    }

    // Step 1: Attempt to fulfill the allocation from existing preallocated blocks.
    unsigned remaining_len = try_prealloc(node, offset, len);

    // If the entire request was satisfied, our job is done.
    if (remaining_len == 0) {
        return;
    }

    // Step 2: If blocks remain, fall back to a new, larger allocation.
    // The offset for this new allocation starts after what was preallocated.
    unsigned new_alloc_offset = offset + (len - remaining_len);

    // Normalize the required length up to the next multiple of ALLOC_GRANULARITY.
    unsigned normalized_len = ((remaining_len + ALLOC_GRANULARITY - 1) / ALLOC_GRANULARITY) * ALLOC_GRANULARITY;
    
    // Allocate the contiguous block of memory.
    unsigned char *data_ptr = malloc_contigous_pages(normalized_len);
    if (data_ptr == NULL) {
        // Allocation failed. In a real kernel, this would propagate an error.
        return;
    }

    // Create an extent for the portion we actually needed for this call.
    create_extent(node, new_alloc_offset, remaining_len, data_ptr);

    // Step 3: If the normalized allocation resulted in extra space, create a
    // new prealloc block to hold it for future allocations.
    unsigned extra_len = normalized_len - remaining_len;
    if (extra_len > 0) {
        Prealloc *new_pa = (Prealloc *)malloc(sizeof(Prealloc));
        if (new_pa == NULL) {
            // Failed to create prealloc struct. The extra memory is now effectively
            // leaked, as it is not tracked anywhere.
            return;
        }

        // The new Prealloc block represents the *entire* chunk obtained from
        // malloc_contigous_pages. The first part is considered 'used' by the
        // extent we just created, and the rest is 'free'.
        new_pa->pa_lstart = new_alloc_offset;
        new_pa->pa_data = data_ptr;
        new_pa->pa_len = normalized_len;
        new_pa->pa_free = extra_len;

        // Prepend the new prealloc block to the inode's list for quick access.
        new_pa->next = node->preallocs;
        node->preallocs = new_pa;
    }
}