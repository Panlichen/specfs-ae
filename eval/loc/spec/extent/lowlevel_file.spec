[OPTIMIZATION]
Now generate new C code to update the previous generated C code to support similar "extent feature" (similar in ext4).
- An extent is simply a set of pages which are logically contiguous within the file and also on the underlying page device.
- For a sequence of continuous pages, `extent` has the start number of pages and the number of the continuous pages. 
- With extent, according to the start address of the extension, the filesystem can access all the pages in the extension,
so the inode no longer stores the address of each page, thus saving space in the metadata.
- If a file has multiple extents, it should be constructed as a linked list in the inode data structure.
- The contiguous read/write operations can be merged for better performance. 


[PROMPT]
If needed, any modification to the rely data structure or new functions are welcomed.
Provide complete `lowlevel_file.h` and `lowlevel_file.c` files that:
- If you modify a function in [Guarantee], reflect the modified function directly in the file
- If you modify a data structure or function in [Rely], write the new implementation to the files as well.
- Ensure memory safety.

[Recommendation]
The followings are some recommend modifications to the [Rely] and [Guarantee]

[Rely]
- `inode`: the indextb should be replaced by a new `extent` data structure.

[Guarantee]
- `file_allocate`: when allocating pages, if possible, allocate continuous pages to form an extent.
- `file_read`: get the pages from the file (inode)'s extent, read the data from the pages
- `file_write`: get the pages from the file (inode)'s extent, write the data to the file. Allocate new pages (or extent) if the file requires more pages. 

