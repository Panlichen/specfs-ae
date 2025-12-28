/*
 * [Module Name] Main_Dir
 * [Rely] Path_Handling, Directory_OPs and Commons
 */

 #include <assert.h>
 #include <stdlib.h>

 #include "main_dir.h"
 
 
 
 int atomfs_ins(char* path[], char* name, int mode, unsigned maj, unsigned min)
 {
     struct inode* cur;
     struct inode* inum;
     unsigned temp;
 
     lock(root_inum);
     cur = locate(root_inum, path);
     temp = check_ins(cur , name);
     if(temp)
         return -1;
     inum = malloc_inode(mode, maj, min);
     insert(cur,inum, name );
     unlock(cur);
     return 0;
 }
 
 
 int atomfs_del(char* path[], char* name)
 {
     struct inode* cur;
     struct inode* inum;
     unsigned temp;
     lock( root_inum );
     cur = locate(root_inum, path);
     temp = check_del(cur, name);
     if( temp )
         return -1;
     inum = delete(cur,name);
     unlock( cur );
     dispose_inode( inum );
     return 0;
 }
 
 
 int check_dir(struct inode* inum)
 {
     if ( inum == NULL )
          return 1;
     if( inum->mode != DIR_MODE )
     {
         unlock( inum );
         return 1;
     }
     return 0;
 }
 
 void filldir(struct inode* inum, char** dircontent)
 {
     unsigned i;
     unsigned j;
     struct entry* walk;
     i = 0;
     j = 0;
     while(i < DIRTB_NUM)
     {
         walk = inum->dir->tb[i];
         while(walk != NULL)
         {
             dircontent[j] = malloc_string(walk->name);
             j = j+1;
             walk = walk->next;
         }
         i = i+1;
     }
     dircontent[j] = NULL;
     return;
 }
 
 char** atomfs_readdir(char* path[])
 {
     struct inode* inum;
     char** dircontent;
     int temp;
     lock( root_inum );
     inum = locate(root_inum, path);
     temp = check_dir(inum);
     if (temp == 1)
         return NULL;
     dircontent = malloc_dir_content(inum->size + 1);
     filldir(inum, dircontent);
     unlock(inum);
     return dircontent;
 }
 
 struct getattr_ret* atomfs_getattr(char* path[])
 {
     struct inode* inum;
     struct getattr_ret* ret;
     lock( root_inum );
     inum = locate(root_inum, path);
     if(inum == NULL)
         return NULL;
     ret = malloc_getattr_ret(inum, inum->mode, inum->size, inum->maj, inum->min);
     unlock(inum);
     return ret;
 }