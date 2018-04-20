#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"
#include "process.h" // I added?

static dfs_inode inodes[DFS_INODE_MAX_NUM]; // all inodes
static dfs_superblock sb; // superblock
static uint32 fbv[DFS_MAX_FILESYSTEM_SIZE / DFS_BLOCKSIZE / 32]; // Free block vector

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.
static lock_t f_lock; // lock for allocation and free TODO static or not?

// STUDENT: put your file system level functions below.
// Some skeletons are provided. You can implement additional functions.

///////////////////////////////////////////////////////////////////
// Non-inode functions first
///////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use.
//-----------------------------------------------------------------

void DfsModuleInit() {
// You essentially set the file system as invalid and then open 
// using DfsOpenFileSystem().
  int i;
  
  dbprintf('f', "DfsModuleInit (%d): Entering function\n", GetCurrentPid());

  sb.valid = 0;

  if ((f_lock = LockCreate()) == SYNC_FAIL) {
    printf("DfsModuleInit (%d): Could not create the file lock\n", GetCurrentPid());
    GracefulExit();
  }

  for (i = 0; i < DFS_INODE_MAX_NUM; i++)
    inodes[i].inuse = 0;
  for (i = 0; i < DFS_MAX_FILESYSTEM_SIZE / DFS_BLOCKSIZE / 32; i++)
    fbv[i] = 0;

  DfsOpenFileSystem(); //????????????????????????????????????????????

  /*
  if (DfsOpenFileSystem() != DFS_SUCCESS) {
    printf("DfsModuleInit (%d): Could not open file system\n", GetCurrentPid());
    GracefulExit();
  }*/
  dbprintf('f', "DfsModuleInit (%d): Leaving function\n", GetCurrentPid());
}

//-----------------------------------------------------------------
// DfsInavlidate marks the current version of the filesystem in
// memory as invalid.  This is really only useful when formatting
// the disk, to prevent the current memory version from overwriting
// what you already have on the disk when the OS exits.
//-----------------------------------------------------------------

void DfsInvalidate() {
// This is just a one-line function which sets the valid bit of the 
// superblock to 0.
  dbprintf('f', "DfsInvalidate (%d): Entering function\n", GetCurrentPid());

  sb.valid = 0;
  
  dbprintf('f', "DfsInvalidate (%d): Leaving function\n", GetCurrentPid());
}

//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on 
// failure.
//-------------------------------------------------------------------

int DfsOpenFileSystem() {
  disk_block block;
  dfs_block  inode_block, fbv_block;
  uint32     blocksize, inode_size, fbv_size;

  dbprintf('f', "DfsOpenFileSystem (%d): Entering function\n", GetCurrentPid());
//Basic steps:
// Check that filesystem is not already open
  if (sb.valid) {
    dbprintf('f', "DfsOpenFileSystem (%d): file system already opened.. function failed\n", GetCurrentPid());
    return DFS_FAIL;
  }

// Read superblock from disk.  Note this is using the disk read rather 
// than the DFS read function because the DFS read requires a valid 
// filesystem in memory already, and the filesystem cannot be valid 
// until we read the superblock. Also, we don't know the block size 
// until we read the superblock, either.
  if ((blocksize = DiskReadBlock(1 * 2, &block)) == DISK_FAIL) {
    dbprintf('f', "DfsOpenFileSystem (%d): Could not read file system\n", GetCurrentPid());
    return DFS_FAIL;
  }

// Copy the data from the block we just read into the superblock in memory
  bcopy(block.data, (char *)(&sb), blocksize);

  printf("valid: %d\n", sb.valid);
  printf("fsBlocksize: %d\n", sb.fsBlocksize);
  printf("numFsBlocks: %d\n", sb.numFsBlocks);
  printf("inodeStartBlock: %d\n", sb.inodeStartBlock);
  printf("fbvStartBlock: %d\n", sb.fbvStartBlock);

// All other blocks are sized by virtual block size:
// Read inodes
// Read free block vector
// Change superblock to be invalid, write back to disk, then change 
// it back to be valid in memory

  // Read Inodes
  if ((inode_size = DfsReadBlock(sb.inodeStartBlock, &inode_block)) == DFS_FAIL) {
    dbprintf('f', "DfsOpenFileSystem (%d): Could not read inodes from disk\n", GetCurrentPid());
    return DFS_FAIL;
  }
  bcopy(inode_block.data, (char *)inodes, inode_size);
  
  // Read fbv
  if ((fbv_size = DfsReadBlock(sb.fbvStartBlock, &fbv_block)) == DISK_FAIL) {
    dbprintf('f', "DfsOpenFileSystem (%d): Could not read fbv from disk\n", GetCurrentPid());
    return DFS_FAIL;
  }
  bcopy(fbv_block.data, (char *)fbv, fbv_size);

  // invalidate superblock and write back to disk
  sb.valid = 0;
  bcopy((char *)(&sb), block.data, blocksize);
  if ((blocksize = DiskWriteBlock(1 * 2, &block)) == DISK_FAIL) {
    dbprintf('f', "DfsOpenFileSystem (%d): Could not write file system back to disk\n", GetCurrentPid());
    return DFS_FAIL;
  }
  sb.valid = 1;

  dbprintf('f', "DfsOpenFileSystem (%d): Leaving function\n", GetCurrentPid());
  return DFS_SUCCESS;
}


//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() {
  disk_block block;
  dfs_block  inode_block, fbv_block;
  uint32     blocksize, inode_size, fbv_size;

  dbprintf('f', "DfsCloseFileSystem (%d): Entering function\n", GetCurrentPid());

  if (!sb.valid) {
    dbprintf('f', "DfsCloseFileSystem (%d): Cannot write an invalid file system\n", GetCurrentPid());
    return DFS_FAIL;
  }

  bcopy((char *)(&sb), block.data, sb.fsBlocksize); 
  if ((blocksize = DiskWriteBlock(1 /*blocknum*/, &block)) == DISK_FAIL) {
    dbprintf('f', "DfsCloseFileSystem (%d): Could not write file system back to disk\n", GetCurrentPid());
    return DFS_FAIL;
  }
  
  // TODO write inode and fbv to disk as well?
  // TODO need a for loop for inode and fbv as well in case they are larger than one block
  bcopy((char *)inodes, inode_block.data, sb.fsBlocksize);
  if ((inode_size = DfsWriteBlock(sb.inodeStartBlock, &inode_block)) == DFS_FAIL) {
    dbprintf('f', "DfsCloseFileSystem (%d): Could not write inode blocks back to disk\n", GetCurrentPid());
    return DFS_FAIL;
  }

  bcopy((char *)fbv, fbv_block.data, sb.fsBlocksize);
  if ((fbv_size = DfsWriteBlock(sb.fbvStartBlock, &fbv_block)) == DFS_FAIL) {
    dbprintf('f', "DfsCloseFileSystem (%d): Could not write fbv blocks back to disk\n", GetCurrentPid());
    return DFS_FAIL;
  }  

  sb.valid = 0;
 
  dbprintf('f', "DfsCloseFileSystem (%d): Leaving function\n", GetCurrentPid());
  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------

int DfsAllocateBlock() {
// Check that file system has been validly loaded into memory
// Find the first free block using the free block vector (FBV), mark it in use
// Return handle to block
  int i, j;
  uint32 fbv_bunch, fbv_bit, fbv_idx;
  
  dbprintf('f', "DfsAllocateBlock (%d): Entering function\n", GetCurrentPid());

  if (!sb.valid) {
    dbprintf('f', "DfsAllocateBlock (%d): Can't free from invalid file system\n", GetCurrentPid());
    return DFS_FAIL;
  }
  
  if (LockHandleAcquire(f_lock) != SYNC_SUCCESS) {
    dbprintf('f', "DfsAllocateBlock (%d): Could not aquire the file lock\n", GetCurrentPid());
    return DFS_FAIL;
  }

  for (i = 0; i < sb.numFsBlocks / 32; i++) {
    if (!fbv[i]) {
      fbv_bunch = fbv[i];
      fbv_idx = i;
      break;
    }
  }

  for (j = 0; j < 32; j++) {
    if (fbv_bunch & 0x1) {
      fbv_bit = j;
      break;
    }
    fbv_bunch = fbv_bunch >> 0x1;
  }
  
  // set allocated page bit to 0
  if (fbv_bit == 31)
    fbv[fbv_idx] = 0x0;
  else
    fbv[fbv_idx] = fbv[fbv_idx] & (negativeone << (fbv_bit + 1) | ((1 << fbv_bit) - 1));

  if (LockHandleRelease(f_lock) != SYNC_SUCCESS) {
    dbprintf('f', "DfsAllocateBlock (%d): Could not release the file lock\n", GetCurrentPid());
    return DFS_FAIL;
  }

  dbprintf('f', "DfsAllocateBlock (%d): Leaving function\n", GetCurrentPid());

  return fbv_idx * 32 + fbv_bit;
}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------

int DfsFreeBlock(uint32 blocknum) {
  uint32 fbv_idx, fbv_bit, fbv_mask;
  
  dbprintf('f', "DfsFreeBlock (%d): Entering function\n", GetCurrentPid());

  if (!sb.valid) {
    dbprintf('f', "DfsFreeBlock (%d): Can't free from invalid file system\n", GetCurrentPid());
    return DFS_FAIL;
  }

  if (LockHandleAcquire(f_lock) != SYNC_SUCCESS) {
    dbprintf('f', "DfsFreeBlock (%d): Could not aquire the file lock\n", GetCurrentPid());
    return DFS_FAIL;
  }

  fbv_idx = blocknum / 32;
  fbv_bit = blocknum - fbv_idx * 32;
  fbv_mask = 1 << fbv_bit;

  fbv[fbv_idx] |= fbv_mask;

  if (LockHandleRelease(f_lock) != SYNC_SUCCESS) {
    dbprintf('f', "DfsFreeBlock (%d): Could not release the file lock\n", GetCurrentPid());
    return DFS_FAIL;
  }
  
  dbprintf('f', "DfsFreeBlock (%d): Leaving function\n", GetCurrentPid());

  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------

int DfsReadBlock(uint32 blocknum, dfs_block *b) {
  uint32 blocksize;
  uint32 totalsize;
  int i;
  //TODO what does it mean by it could span multiple physical disk blocks? 
  dbprintf('f', "DfsReadBlock (%d): Entering function\n", GetCurrentPid());
  /*if (!(fbv[blocknum / 32] & (1 < blocknum % 32))) {
    dbprintf('f', "DfsReadBlock (%d): Block was not previously allocated, cannot read\n", GetCurrentPid());
    return DFS_FAIL;
  }*/

  for (i = 0; i < (sb.fsBlocksize / DISK_BLOCKSIZE); i++) {
    printf("reading blocknum %d\n", blocknum * 2 +i);  
    if ((blocksize = DiskReadBlock(blocknum * 2 + i, (disk_block *)(b + i * DISK_BLOCKSIZE))) == DISK_FAIL) {
      dbprintf('f', "DfsReadBlock (%d): Could not read block from disk\n", GetCurrentPid());
      return DFS_FAIL;
    }
    totalsize += blocksize;
  }

  dbprintf('f', "DfsReadBlock (%d): Leaving function\n", GetCurrentPid());
  return totalsize;
}


//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------

int DfsWriteBlock(uint32 blocknum, dfs_block *b){
  uint32 blocksize; 
  int i;

  dbprintf('f', "DfsWriteBlock (%d): Entering function\n", GetCurrentPid());
  /*if (!(fbv[blocknum / 32] & (1 < blocknum % 32))) {
    dbprintf('f', "DfsWriteBlock (%d): Block was not previously allocated, cannot write\n", GetCurrentPid());
    return DFS_FAIL;
  }*/
 
  for (i = 0; i < (sb.fsBlocksize / DISK_BLOCKSIZE); i++) {  
    if ((blocksize = DiskWriteBlock(blocknum * 2 + i, (disk_block *)(b + i * DISK_BLOCKSIZE))) == DISK_FAIL) {
      dbprintf('f', "DfsWriteBlock (%d): Could not write block to disk\n", GetCurrentPid());
      return DFS_FAIL;
    }
  }

  dbprintf('f', "DfsWriteBlock (%d): Leaving function\n", GetCurrentPid());
  return blocksize;
}


////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
////////////////////////////////////////////////////////////////////////////////
// TODO check to make sure locks are used in correct places in inode functions
// TODO also check to make sure all error checks are correct
//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for 
// the given filename. If the filename is found, return the handle 
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------

int DfsInodeFilenameExists(char *filename) {
  uint32 handle = -1;
  int    i;

  dbprintf('f', "DfsInodeFilenameExists (%d): Entering function\n", GetCurrentPid());
  if (!sb.valid) {
    dbprintf('f', "DfsInodeFilenameExists (%d): file system is not valid\n", GetCurrentPid());
    return DFS_FAIL;
  }

  for (i = 0; i < DFS_INODE_MAX_NUM; i++) {
    if (inodes[i].inuse && dstrncmp(filename, inodes[i].filename, dstrlen(filename))) {
      handle = i;
    }
  }

  if (handle == -1) {
    dbprintf('f', "DfsInodeFilenameExists (%d): Filename given does not exist in current inodes\n", GetCurrentPid());
    return DFS_FAIL;
  }
    
  dbprintf('f', "DfsInodeFilenameExists (%d): Leaving function\n", GetCurrentPid());
  return handle;
}


//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------

int DfsInodeOpen(char *filename) {
  uint32 handle;
  int    i;

  dbprintf('f', "DfsInodeOpen (%d): Entering function\n", GetCurrentPid());
  if (!sb.valid) {
    dbprintf('f', "DfsInodeOpen (%d): file system is not valid\n", GetCurrentPid());
    return DFS_FAIL;
  }

  handle = DfsInodeFilenameExists(filename);
  
  if (handle == DFS_FAIL) {
    if (LockHandleAcquire(f_lock) == SYNC_FAIL) {
      dbprintf('f', "DfsInodeOpen (%d): could not acquire the file lock\n", GetCurrentPid());
      return DFS_FAIL;
    }

    for (i = 0; i < DFS_INODE_MAX_NUM; i++) { //TODO check to make sure we can use this constant
      if (!inodes[i].inuse) {
        handle = i;
        inodes[i].inuse = 1;
        dstrncpy(inodes[i].filename, filename, dstrlen(filename));
        break;
      }
    }

    if (LockHandleRelease(f_lock) == SYNC_FAIL) {
      dbprintf('f', "DfsInodeOpen (%d): could not release the file lock\n", GetCurrentPid());
      return DFS_FAIL;
    }

    dbprintf('f', "DfsInodeOpen (%d): No more available inodes\n", GetCurrentPid());
    return DFS_FAIL;
  }

  dbprintf('f', "DfsInodeOpen (%d): Leaving function\n", GetCurrentPid());
  return handle;
}


//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode. Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------

int DfsInodeDelete(uint32 handle) {
  
  dbprintf('f', "DfsInodeDelete (%d): Entering function\n", GetCurrentPid());
  if (!sb.valid) {
    dbprintf('f', "DfsInodeDelete (%d): file system is not valid\n", GetCurrentPid());
    return DFS_FAIL;
  }
  if (!inodes[handle].inuse) {
    dbprintf('f', "DfsInodeDelete (%d): inode trying to free is not inuse\n", GetCurrentPid());
    return DFS_FAIL;
  }

  if (LockHandleAcquire(f_lock) == SYNC_FAIL) {
    dbprintf('f', "DfsInodeDelete (%d): could not acquire the file lock\n", GetCurrentPid());
    return DFS_FAIL;
  }

  inodes[handle].inuse = 0;

  // TODO de-allocate indirect and direct? mfree?

  if (LockHandleRelease(f_lock) == SYNC_FAIL) {
    dbprintf('f', "DfsInodeDelete (%d): could not release the file lock\n", GetCurrentPid());
    return DFS_FAIL;
  }
  
  dbprintf('f', "DfsInodeDelete (%d): Leaving function\n", GetCurrentPid());
  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  uint32    index, size;
  dfs_block block;  

  dbprintf('f', "DfsInodeReadBytes (%d): Entering function\n", GetCurrentPid());
  if (!sb.valid) {
    dbprintf('f', "DfsInodeReadBytes (%d): file system is not valid\n", GetCurrentPid());
    return DFS_FAIL;
  }
  if (!inodes[handle].inuse) {
    dbprintf('f', "DfsInodeReadBytes (%d): inode trying to access is not in use\n", GetCurrentPid());
    return DFS_FAIL;
  }

  index = start_byte / sb.fsBlocksize;
  // TODO call translate? Can number of bytes be more than one DFS block?
  if ((size = DfsReadBlock(index, &block)) == DFS_FAIL){
    dbprintf('f', "DfsInodeReadBytes (%d): Could not read the block from the disk\n", GetCurrentPid());
    return DFS_FAIL;
  }
  bcopy(block.data + start_byte % sb.fsBlocksize, (char *)mem, num_bytes);
    
  dbprintf('f', "DfsInodeReadBytes (%d): Leaving function\n", GetCurrentPid());
  return num_bytes;
}


//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to 
// by mem to the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only writing part 
// of a given file system block, you'll need to read that block 
// from the disk first. Return DFS_FAIL on failure and the number 
// of bytes written on success.
//-----------------------------------------------------------------

int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  uint32    index, size;
  dfs_block block;

  dbprintf('f', "DfsInodeWriteBytes (%d): Entering function\n", GetCurrentPid());
  if (!sb.valid) {
    dbprintf('f', "DfsInodeWriteBytes (%d): file system is not valid\n", GetCurrentPid());
    return DFS_FAIL;
  }
  /*index = start_byte / sb.fsBlocksize;
  if (start_byte % sb.fsBlocksize) {
    if ((size = DfsReadBlock(index, &block)) == DFS_FAIL){
      dbprintf('f', "DfsInodeWriteBytes (%d): Could not read the block from the disk\n", GetCurrentPid());
      return DFS_FAIL;
    }
    bcopy(block.data + start_byte % sb.fsBlocksize, (char *)mem, num_bytes);
  } else {
    bcopy((char *)mem, block.data + start_byte % sb.fsBlocksize, num_bytes);
    if ((size = DfsWriteBlock(index, &block)) == DFS_FAIL){
      dbprintf('f', "DfsInodeWriteBytes (%d): Could not write the block to the disk\n", GetCurrentPid());
      return DFS_FAIL;
    }
  }*/

  return num_bytes;
}


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

int DfsInodeFilesize(uint32 handle) {
  dbprintf('f', "DfsInodeFilesize (%d): Entering function\n", GetCurrentPid());
  if (!sb.valid) {
    dbprintf('f', "DfsInodeFilesize (%d): file system is not valid\n", GetCurrentPid());
    return DFS_FAIL;
  }
  if (!inodes[handle].inuse) {
    dbprintf('f', "DfsInodeFilesize (%d): inode trying to access is not in use\n", GetCurrentPid());
    return DFS_FAIL;
  }

  dbprintf('f', "DfsInodeFilesize (%d): Leaving function\n", GetCurrentPid());

  return inodes[handle].fileSize;
}


//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block 
// for the given inode, storing its blocknumber at index 
// virtual_blocknumber in the translation table. If the 
// virtual_blocknumber resides in the indirect address space, and 
// there is not an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file system 
// block number on success.
//-----------------------------------------------------------------

int DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {

  int fs_block;

  dbprintf('f', "DfsInodeAllocateVirtualBlock (%d): Entering function\n", GetCurrentPid());
  
  if (!sb.valid) {
    dbprintf('f', "DfsInodeFilesize (%d): file system is not valid\n", GetCurrentPid());
    return DFS_FAIL;
  }
  if (!inodes[handle].inuse) {
    dbprintf('f', "DfsInodeFilesize (%d): inode trying to access is not in use\n", GetCurrentPid());
    return DFS_FAIL;
  }

  // check if fs_block is valid
  if((fs_block = DfsAllocateBlock()) == DFS_FAIL) {
    dbprintf('f', "DfsInodeAllocateVirtualBlock (%d): unable to allocate fs block\n", GetCurrentPid());
    return DFS_FAIL;
  }
 
  // find an unused index in directAddr table
  //directAddr[virtual_blocknum] = fs_block;

  return DFS_SUCCESS;

}



//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

int DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) {

  return DFS_SUCCESS;
}
