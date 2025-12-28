#include "delalloc.h"
#include "util.h"
#include <string.h>

struct alloc_buffer alloc_buffer;

int data_write_count = 0;
int data_read_count = 0;

static void bflush() {
    buffer_lock(&alloc_buffer);
    void *buf[128];
    for (unsigned int i = 0; i < alloc_buffer.size; ++i) {
        struct record *rec = &alloc_buffer.rec[i];
        buf[i] = rec->tb->index[rec->page];
    }
    // Sort the buffer pointers in ascending order
    for (unsigned int i = 0; i < alloc_buffer.size; ++i) {
        for (unsigned int j = i + 1; j < alloc_buffer.size; ++j) {
            if (buf[i] > buf[j]) {
                void *temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }

    // Count data writes based on physical adjacency
    int write_ops = 0;
    unsigned int start_idx = 0;

    // Skip initial NULL pointers and count them as one write operation if any
    while (start_idx < alloc_buffer.size && buf[start_idx] == NULL) {
        start_idx++;
    }
    if (start_idx > 0) {
        write_ops = 1;
    }

    // Count the rest based on adjacency
    if (start_idx < alloc_buffer.size) {
        write_ops++;  // Start the first non-NULL write operation
        for (unsigned int i = start_idx + 1; i < alloc_buffer.size; ++i) {
            if (buf[i] != (char*)buf[i-1] + PAGE_SIZE) {
                write_ops++;
            }
        }
    }

    data_write_count += write_ops;

    for (unsigned int i = 0; i < alloc_buffer.size; ++i) {
        struct record *rec = &alloc_buffer.rec[i];
        if (!rec->tb->index[rec->page])
            rec->tb->index[rec->page] = malloc_page();
        memcpy(rec->tb->index[rec->page], alloc_buffer.buf + rec->buf_offset, PAGE_SIZE);
    }
    alloc_buffer.size = 0;
    buffer_unlock(&alloc_buffer);
}

unsigned char *bread(struct indextb *tb, unsigned int page) {
    buffer_lock(&alloc_buffer);
    for (unsigned int i = 0; i < alloc_buffer.size; ++i) {
        struct record *rec = &alloc_buffer.rec[i];
        if (rec->tb == tb && rec->page == page) {
            unsigned char *data = alloc_buffer.buf + rec->buf_offset;
            buffer_unlock(&alloc_buffer);
            return data;
        }
    }
    unsigned char *index_data = tb->index[page];
    if (index_data) {
        data_read_count++;
    }
    buffer_unlock(&alloc_buffer);
    return index_data;
}

void bwrite(struct indextb *tb, unsigned int page, unsigned int offset, unsigned char *buf, unsigned int size) {
    buffer_lock(&alloc_buffer);
    for (unsigned int i = 0; i < alloc_buffer.size; ++i) {
        struct record *rec = &alloc_buffer.rec[i];
        if (rec->tb == tb && rec->page == page) {
            unsigned char *buffer = alloc_buffer.buf + rec->buf_offset;
            if (buf)
                memcpy(buffer + offset, buf, size);
            else
                memset(buffer + offset, 0, size);
            buffer_unlock(&alloc_buffer);
            return;
        }
    }

    if (alloc_buffer.size == ALLOC_BUFFER_BLOCK_NUM) {
        buffer_unlock(&alloc_buffer);
        bflush();
        buffer_lock(&alloc_buffer);
    }

    struct record *new_rec = &alloc_buffer.rec[alloc_buffer.size];
    new_rec->tb = tb;
    new_rec->page = page;
    new_rec->buf_offset = alloc_buffer.size * PAGE_SIZE;
    unsigned char *dest = alloc_buffer.buf + new_rec->buf_offset;

    if (tb->index[page]) {
        memcpy(dest, tb->index[page], PAGE_SIZE);
        data_read_count++;
    }
    else
        memset(dest, 0, PAGE_SIZE);

    if (buf)
        memcpy(dest + offset, buf, size);
    else
        memset(dest + offset, 0, size);

    alloc_buffer.size++;
    buffer_unlock(&alloc_buffer);
}

void brels(struct indextb *tb) {
    buffer_lock(&alloc_buffer);
    unsigned int j = 0;
    for (unsigned int i = 0; i < alloc_buffer.size; ++i) {
        if (alloc_buffer.rec[i].tb == tb) continue;
        if (j != i) {
            alloc_buffer.rec[j] = alloc_buffer.rec[i];
            memcpy(alloc_buffer.buf + j * PAGE_SIZE,
                   alloc_buffer.buf + i * PAGE_SIZE,
                   PAGE_SIZE);
            alloc_buffer.rec[j].buf_offset = j * PAGE_SIZE;
        }
        j++;
    }
    alloc_buffer.size = j;
    buffer_unlock(&alloc_buffer);
}