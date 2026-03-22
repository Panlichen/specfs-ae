#include "util.h"

void dispose_inode(struct inode* inum) {
    // Destroy the mutex associated with the inode
    mcs_mutex_destroy(inum->impl);
    
    // Handle different inode types based on mode
    if (inum->mode == DIR_MODE && inum->dir != NULL) {
        // Free directory table for directory inodes
        for (int i = 0; i < DIRTB_NUM; i++) {
            struct entry *entry = inum->dir->tb[i];
            while (entry != NULL) {
                struct entry *next = entry->next;
                free(entry->name);
                free(entry);
                entry = next;
            }
        }
        free(inum->dir);
    } else if (inum->mode == FILE_MODE && inum->file != NULL) {
        // Free index table for file inodes
        for (int i = 0; i < INDEXTB_NUM; i++) {
            if (inum->file->index[i] != NULL) {
                free(inum->file->index[i]);
            }
        }
        free(inum->file);
    }
    
    // Free the inode structure itself
    free(inum);
}