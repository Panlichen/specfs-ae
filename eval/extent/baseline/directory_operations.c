#include "directory_operations.h"
#include <pthread.h> // For thread functions if needed

#include "common.h"

struct counter cnt = { .size = 0 };

void free_entry(struct entry* en)
{
    free(en->name);
    free(en);
    return;
}

void insert(struct inode* cur, struct inode* inum, char* name)
{
    struct entry* new_entry;
    unsigned hashval;

    hashval = hash_func(name);
    new_entry = malloc_entry();
    new_entry->name = malloc_string(name);
    new_entry->inum = inum;
    new_entry->next = cur->dir->tb[hashval];
    cur->dir->tb[hashval] = new_entry;
    cur->size = cur->size + 1;


    //  add counter
    cnt.files[cnt.size] = inum;
    cnt.size = cnt.size + 1;
    if(cnt.size >= MAX_DIR_SIZE)
    {
        fprintf(stderr, "Directory size exceeded\n");
        exit(1);
    }

    return;
}

struct inode* delete(struct inode* inum, char* name)
{
    struct entry* walk;
    struct entry* next_walk;
    struct inode* ret;
    struct entry** hd;
    struct entry* cur;
    unsigned int hashval;
    unsigned cmp;

    if(inum->dir == NULL)
        return NULL;
    hashval = hash_func(name);
    hd = &(inum->dir->tb[hashval]);
    cur = *hd;
    if(cur == NULL)
        return NULL;
    cmp = strcmp(cur->name, name);
    if(cmp == 0)
    {
        ret = cur->inum;
        *hd = cur->next;
        free_entry(cur);
        inum->size = inum->size - 1;
        return ret;
    }
    walk = cur;
    next_walk = walk->next;
    while(next_walk != NULL)
    {
        cmp = strcmp(next_walk->name, name);
        if(cmp==0)
        {
            walk->next = next_walk->next;
            ret = next_walk->inum;
            free_entry(next_walk);
    		inum->size = inum->size - 1;
            return ret;
        }
        walk = next_walk;
        next_walk = next_walk->next;
    }
    return NULL;
}

int check_ins(struct inode *cur , char* name)
{
    struct inode* inum;
    if ( cur == NULL )
        return 1;
    if( cur->mode != DIR_MODE )
    {
        unlock(cur);
        return 1;
    }
    if(cur->size >= MAX_DIR_SIZE)
    {
        unlock(cur);
        return 1;
    }
    inum = find (cur->dir, name);
    if( inum != NULL )
    {
        unlock(cur);
        return 1;
    }
    return 0;
}


int check_del(struct inode* cur, char* name)
{
    struct inode* inum ;
    if( cur == NULL )
        return 1;
    inum = find(cur->dir , name);
    if( inum == NULL )
    {
        unlock( cur );
        return 1;
    }
    lock( inum );
    if((inum->mode == DIR_MODE) && (inum->size > 0))
    {
        unlock( inum );
        unlock( cur );
        return 1;
    }
    return 0;
}
