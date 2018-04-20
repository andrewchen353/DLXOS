#ifndef __DFS_SHARED__
#define __DFS_SHARED__

#define DFS_BLOCKSIZE  1024  // Must be an integer multiple of the disk blocksize
#define NUM_ADDR_BLOCK 10
#define FILENAME_SIZE  44
#define DFS_INODE_MAX_NUM 192

typedef struct dfs_superblock {
  // STUDENT: put superblock internals here
  uint32 valid;
  uint32 fsBlocksize;       // DFS block size in bytes
  uint32 numFsBlocks;       // total number of DFS blocks in file system
  uint32 inodeStartBlock;   // starting block number of array of nodes
  uint32 fbvStartBlock;     // starting block number of free block vector
} dfs_superblock;

typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;

typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 96 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 96 bytes.
  uint32 inuse;
  uint32 fileSize;
  char   filename[FILENAME_SIZE];
  uint32 directAddr[NUM_ADDR_BLOCK];
  uint32 indirectAddr;
} dfs_inode;

#define DFS_MAX_FILESYSTEM_SIZE 0x1000000  // 16MB

#define DFS_FAIL -1
#define DFS_SUCCESS 1

#endif
