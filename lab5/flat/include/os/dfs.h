#ifndef __DFS_H__
#define __DFS_H__

#include "dfs_shared.h"

// non-inode functions
void DfsModuleInit();
int DfsOpenFileSystem();
int DfsCloseFileSystem();
void DfsInvalidate();
int DfsAllocateBlock();
int DfsFreeBlock(int blocknum);
int DfsReadBlock(int blocknum, dfs_block *b);
int DfsWriteBlock(int blocknum, dfs_block *b);

///Inode functions
int DfsInodeDelete();
int DfsInodeReadBytes();
int DfsInodeFilenameExists(char* filename);
int DfsInodeOpen(char* filename);
int DfsInodeDelete(int handle);
int DfsInodeReadBytes(int handle, void* mem, int start_byte, int num_bytes);
int DfsInodeWriteBytes(int handle, void* mem, int start_byte, int num_bytes);
int DfsInodeFilesize(int handle);
int DfsInodeAllocateVirtualBlock(int handle, int virtual_blocknum);
int DfsInodeTranslateVirtualToFilesys(int handle, int virtual_blocknum);

#endif
