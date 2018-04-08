#ifndef __NEWFS_H__
#define __NEWFS_H__

typedef unsigned int uint32;

#include "dfs_shared.h" // This gets us structures and #define's from main filesystem driver

#define NEWFS_INODE_BLOCK_START 1 // Starts after super block (which is in file system block 0, physical block 1)
#define NEWFS_INODE_NUM_BLOCKS 16 // Number of file system blocks to use for inodes
//#define NEWFS_NUM_INODES  //STUDENT: define this
//#define NEWFS_FBV_BLOCK_START //STUDENT: define this
#define NEWFS_BOOT_FILESYSTEM_BLOCKNUM 0 // Where the boot record and superblock reside in the filesystem

#ifndef NULL
#define NULL (void *)0x0
#endif

//STUDENT: define additional parameters here, if any
#define DFS_INODE_MAX_NUM 192
#define DFS_FBV_MAX_NUM_WORS 10

#endif
