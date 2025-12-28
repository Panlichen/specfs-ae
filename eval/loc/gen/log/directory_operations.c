void insert(struct inode* cur, struct inode* inum, char* name)
{
    struct entry* new_entry;
    unsigned int hashval;

    // Get the writable version of the current inode from the log.
    struct inode* logged_cur = log_get(cur, LOG_TYPE_INODE);

    hashval = hash_func(name);
    new_entry = malloc_entry();
    new_entry->name = malloc_string(name);
    new_entry->inum = inum;

    // Modify the logged version of the inode.
    new_entry->next = logged_cur->dir->tb[hashval];
    logged_cur->dir->tb[hashval] = new_entry;
    logged_cur->size = logged_cur->size + 1;
}

struct inode* delete(struct inode* inum, char* name)
{
    struct entry* walk;
    struct entry* next_walk;
    struct inode* ret;
    struct entry** hd;
    struct entry* cur;
    unsigned int hashval;

    // Get the writable version of the inode from the log.
    struct inode* logged_inum = log_get(inum, LOG_TYPE_INODE);

    if (logged_inum->dir == NULL) {
        return NULL;
    }

    hashval = hash_func(name);
    hd = &(logged_inum->dir->tb[hashval]);
    cur = *hd;

    if (cur == NULL) {
        return NULL;
    }

    // Check if the first entry in the chain is the one to delete.
    if (strcmp(cur->name, name) == 0) {
        ret = cur->inum;
        *hd = cur->next;
        log_delete(cur, LOG_TYPE_DIR); // Add entry to the delete log.
        logged_inum->size = logged_inum->size - 1;
        return ret;
    }

    // Traverse the rest of the chain.
    walk = cur;
    next_walk = walk->next;
    while (next_walk != NULL) {
        if (strcmp(next_walk->name, name) == 0) {
            walk->next = next_walk->next;
            ret = next_walk->inum;
            log_delete(next_walk, LOG_TYPE_DIR); // Add entry to the delete log.
            logged_inum->size = logged_inum->size - 1;
            return ret;
        }
        walk = next_walk;
        next_walk = next_walk->next;
    }

    return NULL; // Entry not found.
}