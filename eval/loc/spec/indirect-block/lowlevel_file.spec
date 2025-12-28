[Optimization]
Now generate new C code to update the previous generated C code to support **single indirect pages** for this file system.

[SPECIFICATION of file_allocate]
**Precondition**: there exists an inode 
**Postcondition**: ensure pages exist for [offset, offset + len). When the file grows that an inode needs more pages than INDEXTB_NUM, the last page of the file is set the "indirect page", which does not store actual file contents, only saving the address (or offset) of additional pages. So after allocation, the addresses of the additional pages should be written into the indirect block.

[SPECIFICATION of file_read]
**Precondition**: there exists an inode 
**Postcondition**: read data from pages. If it requires reading additional pages, it first read the indirect page and acquire additional page numbers/addresses from it. It then read those additional pages with the page numbers.

[SPECIFICATION of file_write]
**Precondition**: there exists an inode 
**Postcondition**: write data to pages. If it requires writing to additional pages, it first read the indirect page and acquire additional page numbers/addresses from it, and access those additional pages with the page numbers.


[PROMPT]
If needed, any modification to the rely data structure or new functions are welcomed.
Provide complete `lowlevel_file.h` and `lowlevel_file.c` files that:
- If you modify a function in [Guarantee], reflect the modified function directly in the file
- If you modify a data structure or function in [Rely], write the new implementation to the files as well. 
- If possible, choose the implementation that does not need to modify the [Rely] and [Guarantee].
- Ensure memory safety.


