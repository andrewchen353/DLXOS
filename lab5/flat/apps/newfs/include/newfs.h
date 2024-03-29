#ifndef __NEWFS_H__
#define __NEWFS_H__

typedef unsigned int uint32;

#include "dfs_shared.h" // This gets us structures and #define's from main filesystem driver

#define NEWFS_INODE_BLOCK_START 2 // Starts after super block (which is in file system block 0, physical block 1)
#define NEWFS_INODE_NUM_BLOCKS 18 // Number of file system blocks to use for inodes
#define DFS_INODE_MAX_NUM 192 // STUDENT Defined
#define NEWFS_NUM_INODES DFS_INODE_MAX_NUM //STUDENT: define this
#define NEWFS_FBV_BLOCK_START 20 //STUDENT: define this
#define NEWFS_BOOT_FILESYSTEM_BLOCKNUM 0 // Where the boot record and superblock reside in the filesystem

#ifndef NULL
#define NULL (void *)0x0
#endif

//STUDENT: define additional parameters here, if any
#define FBV_SIZE (DFS_MAX_FILESYSTEM_SIZE/DFS_BLOCKSIZE)/32 //512

#endif
