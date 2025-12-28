#include "util.hh"

/**
 * @brief Populates a string array with the names of all entries in a directory.
 *
 *
 * @param[in]  inum A valid pointer to an initialized inode structure representing
 *                  the directory. The `inum->dir` field must point to a valid
 *                  directory table (`dirtb`).
 * @param[out] dircontent A pointer to an array of char* that will be filled with
 *                        pointers to the newly allocated entry names. It must
 *                        have sufficient capacity to hold all entry names plus
 *                        one NULL terminator.
 */
void fill_dir(struct inode* inum, char** dircontent) {
    int i, j = 0;
    
    for (i = 0; i < DIRTB_NUM; i++) {
        struct entry* walk = inum->dir->tb[i];

        while (walk != NULL) {
            dircontent[j++] = malloc_string(walk->name);
            
            walk = walk->next;
        }
    }
    
    dircontent[j] = NULL;
}