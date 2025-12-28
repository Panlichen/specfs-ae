 #ifndef MAIN_DIR_H_
 #define MAIN_DIR_H_

 #include "common.h"
 #include "util.h"
 
 #include "path_handling.h"
 #include "directory_operations.h"

 
 
 int atomfs_ins(char* path[], char* name, int mode, unsigned maj, unsigned min);
 int atomfs_del(char* path[], char* name);
 char** atomfs_readdir(char* path[]);
 struct getattr_ret* atomfs_getattr(char* path[]);
 
 #endif
 