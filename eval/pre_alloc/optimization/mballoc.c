#include "mballoc.h"
#include "util.h"
#include <stdlib.h>


static unsigned try_prealloc(struct inode *node, unsigned offset, unsigned len) {
    struct Prealloc **prev_next = &node->preallocs;
    struct Prealloc *curr = node->preallocs;
    unsigned remaining = len;

    while (curr != NULL && remaining > 0) {
        unsigned free_start = curr->pa_lstart + (curr->pa_len - curr->pa_free);
        unsigned free_end = free_start + curr->pa_free;

        if (offset >= free_start && offset < free_end) {
            unsigned max_alloc = free_end - offset;
            unsigned alloc = (max_alloc < remaining) ? max_alloc : remaining;
            if (alloc > curr->pa_free) {
                alloc = curr->pa_free;
            }

            create_extent(node, offset, alloc, curr->pa_data + (offset - curr->pa_lstart) * PAGE_SIZE);
            curr->pa_free -= alloc;
            remaining -= alloc;

            if (curr->pa_free == 0) {
                *prev_next = curr->next;
                free(curr);
                curr = *prev_next;
            } else {
                prev_next = &curr->next;
                curr = curr->next;
            }
        } else {
            prev_next = &curr->next;
            curr = curr->next;
        }
    }

    return remaining;
}

static void create_extent(struct inode *node, unsigned start, unsigned length, unsigned char *data) {
    struct Extent *new_extent = malloc(sizeof(struct Extent));
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

void mballoc(struct inode *node, unsigned offset, unsigned len) {
    unsigned remaining = try_prealloc(node, offset, len);

    if (remaining > 0) {
        unsigned granularity = ALLOC_GRANULARITY;
        unsigned normalized = ((remaining + granularity - 1) / granularity) * granularity;
        unsigned char *data = malloc_contigous_pages(normalized);
        if (data == NULL) {
            return;
        }

        unsigned start = offset + (len - remaining);
        create_extent(node, start, remaining, data);

        if (normalized > remaining) {
            unsigned extra = normalized - remaining;
            struct Prealloc *pa = malloc(sizeof(struct Prealloc));
            pa->pa_lstart = start + remaining;
            pa->pa_data = data + remaining * PAGE_SIZE;
            pa->pa_len = extra;
            pa->pa_free = extra;
            pa->next = node->preallocs;
            node->preallocs = pa;
        }
    }

}