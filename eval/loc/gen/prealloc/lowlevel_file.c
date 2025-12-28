

void file_allocate(struct inode *node, unsigned offset, unsigned len) {
    if (len == 0 || offset + len > MAX_FILE_SIZE) return;

    unsigned start_page = offset / PG_SIZE;
    unsigned end_page = (offset + len - 1) / PG_SIZE;
    end_page = end_page >= INDEXTB_NUM ? INDEXTB_NUM-1 : end_page;

    // Find maximal contiguous unallocated blocks
    unsigned current = start_page;
    while (current <= end_page) {
        if (is_allocated(node, current)) { current++; continue; }

        // Find block length
        unsigned block_start = current;
        while (current <= end_page && !is_allocated(node, current)) current++;
        unsigned block_length = current - block_start;

        mballoc(node, block_start, block_length);
    }

    // Try to merge adjacent extents after allocation
    try_merge_extents(node);
}