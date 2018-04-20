#include "usertraps.h"
#include "misc.h"

#include "newfs.h"

dfs_superblock sb;
dfs_inode inodes[DFS_INODE_MAX_NUM];
uint32 fbv[FBV_SIZE]; // number of entries/32
dfs_block* b;

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

  bzero((char*)&sb, sizeof(dfs_superblock));
  //bzero(inodes, sizeof(dfs_inode) * DFS_INODE_MAX_NUM);
  bzero((char*)fbv, sizeof(uint32) * FBV_SIZE);
  
  // Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
  // sb.valid = 0
  
  sb.valid = 0;

  disksize = disk_size(); // 16MB, max physical disk size
  diskblocksize = disk_blocksize(); // max size in bytes

  sb.fsBlocksize = DFS_BLOCKSIZE;
  sb.numFsBlocks = disksize/DFS_BLOCKSIZE;
  sb.inodeStartBlock = NEWFS_INODE_BLOCK_START;
  sb.fbvStartBlock = NEWFS_FBV_BLOCK_START;

  // Make sure the disk exists before doing anything else
Printf("\nCreating disk\n");
  if (disk_create() == DISK_FAIL) {
    Printf("FATAL ERROR: could not create disk\n");
    Exit();
  }
Printf("\nCreated disk\n");
  // Write all inodes as not in use and empty (all zeros)
  // Next, setup free block vector (fbv) and write free block vector to the disk
  // Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
  // boot record is all zeros in the first physical block, and superblock structure goes into the second physical block
  /*for(i = 0; i < DFS_INODE_MAX_NUM; i++)
    inodes[i] = 0;

  for(i = 0; i < FBV_SIZE; i++)
    fbv[i] = 0xFFFFFFFF;*/

Printf("\nBoot record\n");
  // write boot record to physical disk
  bzero((char*)b, sizeof(dfs_block));
  NewfsWriteBlock(0, b);  
 
Printf("\nSuperblock\n");
  // write superblock to physical disk
  bcopy((char*) &sb, b->data, sizeof(dfs_superblock));
  NewfsWriteBlock(1, b);

  // 0 out all inodes
  bzero((char*)b, sizeof(dfs_block));

Printf("\nInodes\n");
  // write inodes to disk
  for(i = 2; i < 20; i++)
    //bcopy((char*)(inode + i * sizeof(dfs_inode)), b->data, sizeof());
    NewfsWriteBlock(i, b);
  
Printf("\nFBV\n");
  // write free block vector to disk 
  for(i = 20; i < 22; i++)
    NewfsWriteBlock(i, b);

Printf("\nData\n");
  // write data blocks to disk
  for(i = 22; i < sb.numFsBlocks; i++)
    NewfsWriteBlock(i, b);

  sb.valid = 1;

  Printf("newfs (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
}

int NewfsWriteBlock(uint32 blocknum, dfs_block *b) {
  int phys_block1 = blocknum * 2;
  int phys_block2 = phys_block1 + 1;

  if (disk_write_block(phys_block1, b->data) != DISK_SUCCESS)
    return DISK_FAIL;
  if (disk_write_block(phys_block2, (b + disk_blocksize())->data) != DISK_SUCCESS)
    return DISK_FAIL;
  return DISK_SUCCESS;
}
