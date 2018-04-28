#ifndef __FILES_H__
#define __FILES_H__

#include "dfs.h"
#include "files_shared.h"

#define FILE_MODE_R   0x1
#define FILE_MODE_W   0x2
#define FILE_MODE_RW  0x3

#define FILE_MAX_OPEN_FILES 15

void FileModuleInit();
int FileOpen(char* filename, char* mode); 
int FileClose(int handle);
int FileRead(int handle, void* mem, int num_bytes);
int FileWrite(int handle, void* mem, int num_bytes);
int FileSeek(int handle, int num_bytes, int from_where);
int FileDelete(char* filename);
uint32 FileDescFilenameExists(char *filename);

#endif
