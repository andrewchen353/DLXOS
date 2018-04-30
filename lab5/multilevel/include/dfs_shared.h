#ifndef __DFS_SHARED__
#define __DFS_SHARED__

typedef struct dfs_superblock {
  // STUDENT: put superblock internals here
  uint32 valid;
  uint32 blocksize;
  uint32 startblock;
  uint32 fbv_startblock;
  uint32 fbv_numwords;
  uint32 fbv_numblocks;
  uint32 data;
} dfs_superblock;

#define DFS_BLOCKSIZE 1024  // Must be an integer multiple of the disk blocksize

typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;

typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 128 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 128 bytes.
  unsigned char type;
  unsigned char permission;
  unsigned char ownerID;
  unsigned char inuse;
  uint32 num_blocks;
  uint32 size;
  uint32 directAddr[10];
  uint32 indirectAddr;
  char junk[128 - 56];
} dfs_inode;

#define DFS_MAX_FILESYSTEM_SIZE 0x1000000  // 16MB

#define DFS_FAIL -1
#define DFS_SUCCESS 1



#endif
