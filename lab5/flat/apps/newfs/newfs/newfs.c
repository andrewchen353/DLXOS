#include "usertraps.h"
#include "misc.h"

#include "newfs.h"

dfs_superblock sb;
dfs_inode inodes[DFS_INODE_MAX_NUM];
uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0; // These are global in order to speed things up
int disksize = 0;      // (i.e. fewer traps to OS to get the same number)

int NewfsWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk

void main (int argc, char *argv[])
{
	// STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
	// You need to think of the finer details. You can use bzero() to zero out bytes in memory
  int i;
  //Initializations and argc check
  if(argc != 1)
  {
    Printf("Usage: No arguments expected\n");
    Exit();
  }
  bzero(&sb, sizeof(dfs_superblock));
  bzero(inodes, sizeof(dfs_inode) * DFS_INODE_MAX_NUM);
  bzero(fbv, sizeof(uint32) * DFS_FBV_MAX_NUM_WORDS);
  
  // Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
  // sb.valid = 0
  
  sb.valid = 0;

  disksize = 1 << 24; // 16MB, max physical disk size
  diskblocksize = 1024; // max size in bytes
  num_filesystem_blocks = DFS_INODE_MAX_NUM; // max number of inodes

  sb.fsBlockSize = diskblocksize;
  sb.numFsBlocks = num_filesystem_blocks;
  // sb.disksize ??? TODO

  // Make sure the disk exists before doing anything else
  

  // Write all inodes as not in use and empty (all zeros)
  // Next, setup free block vector (fbv) and write free block vector to the disk
  // Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
  // boot record is all zeros in the first physical block, and superblock structure goes into the second physical block
  for(i = 0; i < DFS_INODE_MAX_NUM; i++)
    inodes[i].inuse = 0;
    // inodes[i]. 
  sb.fbvStartBlock = 0;

  // NewfsWriteBlock(0, );
  // NewfsWriteBlock(1, sb);

  Printf("newfs (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
}

int NewfsWriteBlock(uint32 blocknum, dfs_block *b) {
  // STUDENT: put your code here
}
