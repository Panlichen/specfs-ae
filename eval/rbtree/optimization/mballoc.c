#include "mballoc.h"
#include "util.h"
#include "rbtree.h"
#include <stdlib.h>

int pool_access_count = 0;

static void create_extent(struct inode *node, unsigned start, unsigned length, unsigned char *data) {
    struct Extent *new_extent = malloc(sizeof(struct Extent));
    if (!new_extent) return;

    new_extent->start_page = start;
    new_extent->length = length;
    new_extent->data = data;
    new_extent->next = NULL;

    if (node->extents == NULL) {
        node->extents = new_extent;
    } else {
        struct Extent *curr = node->extents;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = new_extent;
    }
}

unsigned try_prealloc(struct inode *node, unsigned offset, unsigned len) {
    unsigned remaining = len;
    if (!node || !len) return remaining;

    // Find the maximum prealloc node with start <= offset
    struct rb_node *rb_node = rb_find_max_lte(node->prealloc_tree, offset);
    if (!rb_node) return remaining;

    pool_access_count++;
    Prealloc *pa = rb_node->prealloc;
    if (!pa) return remaining;

    unsigned free_start = pa->pa_lstart + (pa->pa_len - pa->pa_free);
    unsigned free_end = free_start + pa->pa_free;

    // Check if the requested offset is within this prealloc range
    if (offset >= free_start && offset < free_end) {
        unsigned max_alloc = free_end - offset;
        unsigned alloc = (max_alloc < remaining) ? max_alloc : remaining;
        if (alloc > pa->pa_free) {
            alloc = pa->pa_free;
        }

        create_extent(node, offset, alloc, pa->pa_data + (offset - pa->pa_lstart) * PAGE_SIZE);
        pa->pa_free -= alloc;
        remaining -= alloc;

        // If prealloc is exhausted, remove it from the tree
        if (pa->pa_free == 0) {
            rb_delete(node, rb_node);
            free(pa);
            // free(rb_node);   //  added manually
        }
    }

    return remaining;
}

void mballoc(struct inode *node, unsigned offset, unsigned len) {
    if (!node || !len) return;

    unsigned remaining = try_prealloc(node, offset, len);

    if (remaining > 0) {
        unsigned granularity = ALLOC_GRANULARITY;
        unsigned normalized = ((remaining + granularity - 1) / granularity) * granularity;
        unsigned char *data = malloc_contigous_pages(normalized);
        if (!data) return;

        unsigned start = offset + (len - remaining);
        create_extent(node, start, remaining, data);

        if (normalized > remaining) {
            unsigned extra = normalized - remaining;
            Prealloc *pa = malloc(sizeof(Prealloc));
            if (!pa) {
                // If we can't create prealloc, just keep the allocated blocks
                return;
            }

            pa->pa_lstart = start + remaining;
            pa->pa_data = data + remaining * PAGE_SIZE;
            pa->pa_len = extra;
            pa->pa_free = extra;

            // Insert the new prealloc into the red-black tree
            rb_insert(node, pa);
        }
    }

}