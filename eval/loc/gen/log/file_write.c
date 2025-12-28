void file_write(struct indextb *tb, unsigned offset, unsigned len, const char *data) {
    unsigned current_offset = offset;
    unsigned remaining = len;

    while (remaining > 0) {
        unsigned page = current_offset / PAGE_SIZE;
        unsigned offset_in_page = current_offset % PAGE_SIZE;
        unsigned bytes_in_page = PAGE_SIZE - offset_in_page;
        unsigned bytes_to_copy = (bytes_in_page < remaining) ? bytes_in_page : remaining;

        // Check if the target page is allocated.
        if (tb->index[page] == NULL) {
            // This block preserves the original code's behavior of handling writes
            // to unallocated pages only in the case of a zero-fill (data == NULL).
            if (data == NULL) {
                // We assume malloc_page() is an existing utility to allocate a page.
                tb->index[page] = malloc_page();

                // Get the log pointer for the newly created page.
                void* log_ptr = log_get(tb->index[page], LOG_TYPE_DATA);

                // Write zeros to the location indicated by the log pointer.
                memset(log_ptr + offset_in_page, 0, bytes_to_copy);
            }
            // If the page is NULL and `data` is not NULL, the original code did
            // nothing, so we maintain that behavior.
        } else {
            // The page already exists, so we get its corresponding log pointer.
            void* log_ptr = log_get(tb->index[page], LOG_TYPE_DATA);

            if (data == NULL) {
                // Perform a zero-fill on the log's memory region.
                memset(log_ptr + offset_in_page, 0, bytes_to_copy);
            } else {
                // Copy the user's data to the log's memory region.
                memcpy(log_ptr + offset_in_page, data, bytes_to_copy);
            }
        }

        // Advance pointers and counters for the next iteration.
        if (data != NULL) {
            data += bytes_to_copy;
        }
        current_offset += bytes_to_copy;
        remaining -= bytes_to_copy;
    }
}