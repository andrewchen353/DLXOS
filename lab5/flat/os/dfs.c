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
lock_t f_lock; // fbv lock
lock_t i_lock; // inode lock 
sem_t f_sem; // fbv lock
sem_t i_sem; // fbv lock

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

  /*if ((f_lock = LockCreate()) == SYNC_FAIL) {
    printf("DfsModuleInit (%d): Could not create the fbv lock\n", GetCurrentPid());
    GracefulExit();
  }
  dbprintf('f', "-------------------CREATED f_lock %d-------------------\n", (int)f_lock);

  if ((i_lock = LockCreate()) == SYNC_FAIL) {
    printf("DfsModuleInit (%d): Could not create the inode lock\n", GetCurrentPid());
    GracefulExit();
  }
  dbprintf('f', "-------------------CREATED i_lock %d-------------------\n", (int)i_lock);*/

  /*if ((f_sem = SemCreate(0)) == SYNC_FAIL) {
    printf("DfsModuleInit (%d): Could not create the fbv sem\n", GetCurrentPid());
    GracefulExit();
  }
  dbprintf('f', "-------------------CREATED f_sem %d-------------------\n", (int)f_sem);
  if ((i_sem = SemCreate(0)) == SYNC_FAIL) {
    printf("DfsModuleInit (%d): Could not create the inode sem\n", GetCurrentPid());
    GracefulExit();
  }
  dbprintf('f', "-------------------CREATED i_sem %d-------------------\n", (int)i_sem);*/

  for (i = 0; i < DFS_INODE_MAX_NUM; i++)
    inodes[i].inuse = 0;
  for (i = 0; i < DFS_MAX_FILESYSTEM_SIZE / DFS_BLOCKSIZE / 32; i++)
    fbv[i] = 0xFFFFFFFF;

  DfsOpenFileSystem(); 

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
  disk_block  inode_block, fbv_block;
  uint32     blocksize, inode_size, fbv_size;
  int        i;

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

  dbprintf('f',"DfsOpenFileSystem (%d): filesystem valid: %d\n", GetCurrentPid(), sb.valid);
  dbprintf('f',"DfsOpenFileSystem (%d): filesystem fsBlocksize: %d\n", GetCurrentPid(), sb.fsBlocksize);
  dbprintf('f',"DfsOpenFileSystem (%d): filesystem numFsBlocks: %d\n", GetCurrentPid(), sb.numFsBlocks);
  dbprintf('f',"DfsOpenFileSystem (%d): filesystem inodeStartBlock: %d\n", GetCurrentPid(), sb.inodeStartBlock);
  dbprintf('f',"DfsOpenFileSystem (%d): filesystem fbvStartBlock: %d\n", GetCurrentPid(), sb.fbvStartBlock);

// All other blocks are sized by virtual block size:
// Read inodes
// Read free block vector
// Change superblock to be invalid, write back to disk, then change 
// it back to be valid in memory

  // Read Inodes
  for (i = 0; i < (sb.fbvStartBlock - sb.inodeStartBlock) * (sb.fsBlocksize / DISK_BLOCKSIZE); i++) {
    if ((inode_size = DiskReadBlock(sb.inodeStartBlock * 2 + i, &inode_block)) == DFS_FAIL) {
      dbprintf('f', "DfsOpenFileSystem (%d): Could not read inodes from disk\n", GetCurrentPid());
      return DFS_FAIL;
    }
    bcopy(inode_block.data, (char *)(inodes + i * DISK_BLOCKSIZE), inode_size);
  }
   
  // Read fbv 
  // TODO fix for loop *2
  for (i = 0; i < (sb.numFsBlocks / 32 / DISK_BLOCKSIZE) * 2 * (sb.fsBlocksize / DISK_BLOCKSIZE); i++) {
    printf("reading fbv block: %d\n", sb.fbvStartBlock * 2 + i);
    if ((fbv_size = DiskReadBlock(sb.fbvStartBlock * 2 + i, &fbv_block)) == DFS_FAIL) {
      dbprintf('f', "DfsOpenFileSystem (%d): Could not read fbv from disk\n", GetCurrentPid());
      return DFS_FAIL;
    }
    bcopy(fbv_block.data, (char *)(fbv + i * DISK_BLOCKSIZE), fbv_size);
  }

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
  disk_block inode_block, fbv_block;
  uint32     blocksize, inode_size, fbv_size;
  int        i;

  dbprintf('f', "DfsCloseFileSystem (%d): Entering function\n", GetCurrentPid());

  if (!sb.valid) {
    dbprintf('f', "DfsCloseFileSystem (%d): Cannot write an invalid file system\n", GetCurrentPid());
    return DFS_FAIL;
  }

  bcopy((char *)(&sb), block.data, sizeof(sb)); 
  if ((blocksize = DiskWriteBlock(1 * 2 /*blocknum*/, &block)) == DISK_FAIL) {
    dbprintf('f', "DfsCloseFileSystem (%d): Could not write file system back to disk\n", GetCurrentPid());
    return DFS_FAIL;
  }
  
  // write inodes  
  for (i = 0; i < (sb.fbvStartBlock - sb.inodeStartBlock) * (sb.fsBlocksize / DISK_BLOCKSIZE); i++) {
    bcopy((char *)(inodes + i * DISK_BLOCKSIZE), inode_block.data, DISK_BLOCKSIZE);
    if ((inode_size = DiskWriteBlock(sb.inodeStartBlock * 2 + i, &inode_block)) == DFS_FAIL) {
      dbprintf('f', "DfsCloseFileSystem (%d): Could not write inode blocks back to disk\n", GetCurrentPid());
      return DFS_FAIL;
    }
  }
  
  // write fbv
  for (i = 0; i < (sb.numFsBlocks / 32 / DISK_BLOCKSIZE) * (sb.fsBlocksize / DISK_BLOCKSIZE); i++) {
    bcopy((char *)(fbv + i * DISK_BLOCKSIZE), fbv_block.data, DISK_BLOCKSIZE);
    if ((fbv_size = DiskWriteBlock(sb.fbvStartBlock * 2 + i, &fbv_block)) == DFS_FAIL) {
      dbprintf('f', "DfsCloseFileSystem (%d): Could not write fbv blocks back to disk\n", GetCurrentPid());
      return DFS_FAIL;
    }
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
  
  /*if (LockHandleAcquire(f_lock) == SYNC_FAIL) {
    dbprintf('f', "DfsAllocateBlock (%d): Could not aquire the file lock\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsAllocateBlock ACQUIRED f_lock-------------------\n");*/
  /*if (SemHandleSignal(f_sem) == SYNC_FAIL) {
    dbprintf('f', "DfsAllocateBlock (%d): Could not acquire the file sem\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsAllocateBlock ACQUIRED f_sem-------------------\n");*/

  for (i = 0; i < sb.numFsBlocks / 32; i++) {
    if (fbv[i]) {
      fbv_bunch = fbv[i];
      fbv_idx = i;
      break;
    }
  }

  printf("idx %d, bunch %x\n", fbv_idx, fbv_bunch);

  for (j = 0; j < 32; j++) {
    if (fbv_bunch & 0x80000000) {
      fbv_bit = j;
      break;
    }
    fbv_bunch = fbv_bunch << 0x1;
  }
  printf("new bunch %x, bit %d\n", fbv_bunch, fbv_bit);
  
  // set allocated page bit to 0
  if (fbv_bit == 31)
    fbv[fbv_idx] = 0x0;
  else
    fbv[fbv_idx] = fbv[fbv_idx] & invert(0x1 << (31 - fbv_bit));//(negativeone >> (fbv_bit + 1) | ((1 >> fbv_bit) - 1));

  printf("new fbv %x\n", fbv[fbv_idx]);

  /*if (LockHandleRelease(f_lock) == SYNC_FAIL) {
    dbprintf('f', "DfsAllocateBlock (%d): Could not release the file lock\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsAllocateBlock RELEASED f_lock-------------------\n");*/
  /*if (SemHandleWait(f_sem) == SYNC_FAIL) {
    dbprintf('f', "DfsAllocateBlock (%d): Could not release the file sem\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsAllocateBlock RELEASED f_sem-------------------\n");*/

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

  /*if (LockHandleAcquire(f_lock) == SYNC_FAIL) {
    dbprintf('f', "DfsFreeBlock (%d): Could not aquire the file lock\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsFreeBlock ACQUIRED f_lock-------------------\n");*/
  /*if (SemHandleSignal(f_sem) == SYNC_FAIL) {
    dbprintf('f', "DfsFreeBlock (%d): Could not aquire the file sem\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsFreeBlock ACQUIRED f_lock-------------------\n");*/
  printf("BLOCKNUM %d\n", blocknum);

  fbv_idx = blocknum / 32;
  fbv_bit = blocknum - fbv_idx * 32;
  fbv_mask = 0x80000000 >> fbv_bit;

  fbv[fbv_idx] |= fbv_mask;

  printf("(%d) %x\n", fbv_idx, fbv[fbv_idx]);

  /*if (LockHandleRelease(f_lock) == SYNC_FAIL) {
    dbprintf('f', "DfsFreeBlock (%d): Could not release the file lock\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsFreeBlock RELEASED f_lock-------------------\n");*/
  /*if (SemHandleWait(f_sem) == SYNC_FAIL) {
    dbprintf('f', "DfsFreeBlock (%d): Could not release the file sem\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsFreeBlock RELEASED f_sem-------------------\n");*/
  
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
  uint32 totalsize = 0;
  int i;
  
  dbprintf('f', "DfsReadBlock (%d): Entering function\n", GetCurrentPid());
  printf("fbv value %x\n", fbv[blocknum / 32]);
  if ((fbv[blocknum / 32] & (0x80000000 >> (blocknum % 32)))) {
    dbprintf('f', "DfsReadBlock (%d): Block (%d) was not previously allocated, cannot read\n", GetCurrentPid(), blocknum);
    return DFS_FAIL;
  }

  for (i = 0; i < (sb.fsBlocksize / DISK_BLOCKSIZE); i++) {
    // TODO replace *2
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
  uint32 totalsize = 0; 
  int i;

  dbprintf('f', "DfsWriteBlock (%d): Entering function\n", GetCurrentPid());
  printf("fbv value %x\n", fbv[blocknum / 32]);
  if ((fbv[blocknum / 32] & (0x80000000 >> (blocknum % 32)))) {
    dbprintf('f', "DfsWriteBlock (%d): Block (%d) was not previously allocated, cannot write\n", GetCurrentPid(), blocknum);
    return DFS_FAIL;
  }
 
  for (i = 0; i < (sb.fsBlocksize / DISK_BLOCKSIZE); i++) {  
    if ((blocksize = DiskWriteBlock(blocknum * 2 + i, (disk_block *)(b + i * DISK_BLOCKSIZE))) == DISK_FAIL) {
      dbprintf('f', "DfsWriteBlock (%d): Could not write block to disk\n", GetCurrentPid());
      return DFS_FAIL;
    }
    totalsize += blocksize;
  }

  dbprintf('f', "DfsWriteBlock (%d): Leaving function\n", GetCurrentPid());
  return totalsize;
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
  uint32 handle;
  int    i;

  dbprintf('f', "DfsInodeFilenameExists (%d): Entering function\n", GetCurrentPid());
  if (!sb.valid) {
    dbprintf('f', "DfsInodeFilenameExists (%d): file system is not valid\n", GetCurrentPid());
    return DFS_FAIL;
  }

  for (i = 0; i < DFS_INODE_MAX_NUM; i++) {
    if (inodes[i].inuse && !dstrncmp(inodes[i].filename, filename, dstrlen(filename))) {
      handle = i;
      break;
    }
  }

  if (i == DFS_INODE_MAX_NUM) {
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
    /*if (LockHandleAcquire(i_lock) != SYNC_SUCCESS) {
      dbprintf('f', "DfsInodeOpen (%d): could not acquire the inode lock\n", GetCurrentPid());
      return DFS_FAIL;
    }
    dbprintf('f', "-------------------DfsInodeOpen ACQUIRED i_lock-------------------\n");*/
    /*if (SemHandleSignal(i_sem) == SYNC_FAIL) {
      dbprintf('f', "DfsInodeOpen (%d): could not acquire the inode sem\n", GetCurrentPid());
      return DFS_FAIL;
    }
    dbprintf('f', "-------------------DfsInodeOpen ACQUIRED i_sem-------------------\n");*/

    for (i = 0; i < DFS_INODE_MAX_NUM; i++) {
      if (!inodes[i].inuse) {
        handle = i;
        inodes[i].inuse = 1;
        dstrcpy(inodes[i].filename, filename);
        break;
      }
    }

    if (i == DFS_INODE_MAX_NUM) {
      dbprintf('f', "DfsInodeOpen (%d): No more available inodes\n", GetCurrentPid());
      return DFS_FAIL;
    }

    /*if (LockHandleRelease(i_lock) == SYNC_FAIL) {
      dbprintf('f', "DfsInodeOpen (%d): could not release the inode lock\n", GetCurrentPid());
      return DFS_FAIL;
    }
    dbprintf('f', "-------------------DfsInodeOpen RELEASED i_lock-------------------\n");*/
    /*if (SemHandleWait(i_sem) == SYNC_FAIL) {
      dbprintf('f', "DfsInodeOpen (%d): could not release the inode sem\n", GetCurrentPid());
      return DFS_FAIL;
    }
    dbprintf('f', "-------------------DfsInodeOpen RELEASED i_sem-------------------\n");*/
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
  int i;
  dfs_block block;
  v_table v_t;

  dbprintf('f', "DfsInodeDelete (%d): Entering function\n", GetCurrentPid());
  if (!sb.valid) {
    dbprintf('f', "DfsInodeDelete (%d): file system is not valid\n", GetCurrentPid());
    return DFS_FAIL;
  }
  if (!inodes[handle].inuse) {
    dbprintf('f', "DfsInodeDelete (%d): inode trying to free is not inuse\n", GetCurrentPid());
    return DFS_FAIL;
  }

  /*if (LockHandleAcquire(i_lock) == SYNC_FAIL) {
    dbprintf('f', "DfsInodeDelete (%d): could not acquire the inode lock\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsInodeDelete ACQUIRED i_lock-------------------\n");*/
  /*if (SemHandleSignal(i_sem) == SYNC_FAIL) {
    dbprintf('f', "DfsInodeDelete (%d): could not acquire the inode sem\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsInodeDelete ACQUIRED i_sem-------------------\n");*/

  for (i = 0; i < NUM_ADDR_BLOCK; i++) {
    if (inodes[handle].directAddr[i]) {
      if (DfsFreeBlock(inodes[handle].directAddr[i]) == DFS_FAIL) {
        dbprintf('f', "DfsInodeDelete (%d): Failed to free block\n", GetCurrentPid());
        return DFS_FAIL;
      }
      inodes[handle].directAddr[i] = 0;
    } 
  }

  if (inodes[handle].indirectAddr) {
    if (DfsReadBlock(inodes[handle].indirectAddr, &block) == DFS_FAIL) {
      dbprintf('f', "DfsInodeDelete (%d): Failed to read block\n", GetCurrentPid());
      return DFS_FAIL;
    }
    bcopy((char *)block.data, (char *)v_t.addr, sb.fsBlocksize);
    printf("indirect %d\n", inodes[handle].indirectAddr);
    /*for (i = 0; i < (sb.fsBlocksize / 4); i++) 
      printf("(%d) %d\n", i, v_t.addr[i]);*/
    for (i = 0; i < (sb.fsBlocksize / 4); i++) {
      printf("(%d) value %u\n", i, v_t.addr[i]);
      if (v_t.addr[i] && ((int)v_t.addr[i]) > 0 && ((int)v_t.addr[i]) < sb.numFsBlocks / 32) {
        if (DfsFreeBlock(v_t.addr[i]) == DFS_FAIL) {
          dbprintf('f', "DfsInodeDelete (%d): Failed to free block\n", GetCurrentPid());
          return DFS_FAIL;
       }
        v_t.addr[i] = 0;
      } else {
        printf("NOPE %d\n", i);
      }
    }
    bcopy((char *)v_t.addr, (char *)block.data, sb.fsBlocksize);
    if (DfsWriteBlock(inodes[handle].indirectAddr, &block) == DFS_FAIL) {
      dbprintf('f', "DfsInodeDelete (%d): Failed to write block\n", GetCurrentPid());
      return DFS_FAIL;
    }
    if (DfsFreeBlock(inodes[handle].indirectAddr) == DFS_FAIL) {
      dbprintf('f', "DfsInodeDelete (%d): Failed to free block\n", GetCurrentPid());
      return DFS_FAIL;
    }
    inodes[handle].indirectAddr = 0;
  }

  inodes[handle].inuse = 0;

  /*if (LockHandleRelease(i_lock) == SYNC_FAIL) {
    dbprintf('f', "DfsInodeDelete (%d): could not release the inode lock\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsInodeDelete RELEASED i_lock-------------------\n");*/
  /*if (SemHandleWait(i_sem) == SYNC_FAIL) {
    dbprintf('f', "DfsInodeDelete (%d): could not release the inode sem\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsInodeDelete RELEASED i_sem-------------------\n");*/
  
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
  uint32    index, size, bytes_read, blocknum;
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
  bytes_read = 0;
  while (num_bytes > 0) {
    blocknum = DfsInodeTranslateVirtualToFilesys(handle, index);
    if ((size = DfsReadBlock(blocknum, &block)) == DFS_FAIL){
      dbprintf('f', "DfsInodeReadBytes (%d): Could not read the block from the disk\n", GetCurrentPid());
      return DFS_FAIL;
    }
    if (num_bytes + start_byte % sb.fsBlocksize >= sb.fsBlocksize)
      size = sb.fsBlocksize - start_byte % sb.fsBlocksize;
    else
      size = num_bytes;

    bcopy(block.data + start_byte % sb.fsBlocksize, (char *)(mem + bytes_read), size);
    printf("reading message: %s\n", block.data + start_byte % sb.fsBlocksize);
    printf("read message: %s\n", (char*) (mem +bytes_read));
    num_bytes -= size;
    bytes_read += size;
    start_byte = 0;
    index++;
  }
  inodes[handle].fileSize += bytes_read; 
  dbprintf('f', "DfsInodeReadBytes (%d): Leaving function\n", GetCurrentPid());
  return bytes_read;
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
  uint32    index, size, bytes_written, blocknum;
  dfs_block block;

  dbprintf('f', "DfsInodeWriteBytes (%d): Entering function\n", GetCurrentPid());
  printf("message: %s\n", (char*) mem);
  printf("start byte: %d\n", start_byte);
  printf("num bytes: %d\n", num_bytes);
  if (!sb.valid) {
    dbprintf('f', "DfsInodeWriteBytes (%d): file system is not valid\n", GetCurrentPid());
    return DFS_FAIL;
  }

  if (!inodes[handle].inuse) {
    dbprintf('f', "DfsInodeWriteBytes (%d): inode trying to access is not in use\n", GetCurrentPid());
    return DFS_FAIL;
  }

  index = start_byte / sb.fsBlocksize;
  bytes_written = 0;
  while (num_bytes > 0) {
    blocknum = DfsInodeTranslateVirtualToFilesys(handle, index);
    if ((size = DfsReadBlock(blocknum, &block)) == DFS_FAIL){
      dbprintf('f', "DfsInodeReadBytes (%d): Could not read the block from the disk\n", GetCurrentPid());
      return DFS_FAIL;
    }
    if (num_bytes + start_byte % sb.fsBlocksize >= sb.fsBlocksize)
      size = sb.fsBlocksize - start_byte % sb.fsBlocksize;
    else
      size = num_bytes;

    bcopy((char *)(mem + bytes_written), block.data + start_byte % sb.fsBlocksize, size);
    printf("written message: %s\n", block.data + start_byte % sb.fsBlocksize);

    if ((DfsWriteBlock(blocknum, &block)) == DFS_FAIL){
      dbprintf('f', "DfsInodeWriteBytes (%d): Could not write the block to the disk\n", GetCurrentPid());
      return DFS_FAIL;
    }

    num_bytes -= size;
    bytes_written += size;
    start_byte = 0;
    index++;
  }

  inodes[handle].fileSize += bytes_written; 
  dbprintf('f', "DfsInodeWriteBytes (%d): Leaving function\n", GetCurrentPid());
  return bytes_written;
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
  int fs_block, indirect_table, i;
  dfs_block table;
  v_table v_t;

  dbprintf('f', "DfsInodeAllocateVirtualBlock (%d): Entering function\n", GetCurrentPid());
  
  if (!sb.valid) {
    dbprintf('f', "DfsInodeFilesize (%d): file system is not valid\n", GetCurrentPid());
    return DFS_FAIL;
  }
  if (!inodes[handle].inuse) {
    dbprintf('f', "DfsInodeFilesize (%d): inode trying to access is not in use\n", GetCurrentPid());
    return DFS_FAIL;
  }

  /*if (LockHandleAcquire(i_lock) == SYNC_FAIL) {
    dbprintf('f', "DfsInodeAllocateVirtualBlock (%d): could not acquire the inode lock\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsInodeAllocateVirtualBlock ACQUIRED i_lock-------------------\n");*/
  /*if (SemHandleSignal(i_sem) == SYNC_FAIL) {
    dbprintf('f', "DfsInodeAllocateVirtualBlock (%d): could not acquire the inode sem\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsInodeAllocateVirtualBlock ACQUIRED i_sem-------------------\n");*/

  // check if fs_block is valid
  if((fs_block = DfsAllocateBlock()) == DFS_FAIL) {
    dbprintf('f', "DfsInodeAllocateVirtualBlock (%d): unable to allocate fs block\n", GetCurrentPid());
    return DFS_FAIL;
  }

  printf("handle %d, block num allocated %d\n", handle, fs_block);
 
  // find an unused index in directAddr table
  if (virtual_blocknum < NUM_ADDR_BLOCK)
    if (!inodes[handle].directAddr[virtual_blocknum])
      inodes[handle].directAddr[virtual_blocknum] = fs_block;
    else {
      dbprintf('f', "DfsInodeAllocateVirtualBlock (%d): block trying to allocate is not available\n", GetCurrentPid());
      return DFS_FAIL;
    }
  else
  {
    if (!inodes[handle].indirectAddr) {
      if ((indirect_table = DfsAllocateBlock()) == DFS_FAIL) {
        dbprintf('f', "DfsInodeAllocate (%d): Failed to allocate block\n", GetCurrentPid());
        return DFS_FAIL;
      }
      inodes[handle].indirectAddr = fs_block;
      bzero((char *)v_t.addr, sb.fsBlocksize);
      bcopy((char *)v_t.addr, (char *)table.data, sb.fsBlocksize);
      if ((DfsWriteBlock(inodes[handle].indirectAddr, &table)) == DFS_FAIL) {
        dbprintf('f', "DfsInodeAllocate (%d): Failed to write block\n", GetCurrentPid());
        return DFS_FAIL;
      } 
    } else {
      indirect_table = fs_block;
    }
    if ((DfsReadBlock(inodes[handle].indirectAddr, &table)) == DFS_FAIL) {
      dbprintf('f', "DfsInodeAllocate (%d): Failed to read block\n", GetCurrentPid());
      return DFS_FAIL;
    }
    bcopy((char *)table.data, (char *)v_t.addr, sb.fsBlocksize);
    if (v_t.addr[virtual_blocknum - NUM_ADDR_BLOCK]) {
      dbprintf('f', "DfsInodeAllocate (%d): Invalid address\n", GetCurrentPid());
      return DFS_FAIL;
    }
    printf("value = %u\n",  v_t.addr[virtual_blocknum - NUM_ADDR_BLOCK]);
    v_t.addr[virtual_blocknum - NUM_ADDR_BLOCK] = indirect_table;
    printf("v block allocated = %d\n", virtual_blocknum - NUM_ADDR_BLOCK);
    for (i = 0; i < (sb.fsBlocksize / 4); i++)
      printf("(%d) %u\n", i, v_t.addr[i]);
    bcopy((char *)v_t.addr, (char *)table.data, sb.fsBlocksize);
    if ((DfsWriteBlock(inodes[handle].indirectAddr, &table)) == DFS_FAIL) {
      dbprintf('f', "DfsInodeAllocate (%d): Failed to write block\n", GetCurrentPid());
      return DFS_FAIL;
    }
  }

  /*if (LockHandleRelease(i_lock) == SYNC_FAIL) {
    dbprintf('f', "DfsInodeAllocateVirtualBlock (%d): could not release the inode lock\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsInodeAllocateVirtualBlock RELEASED f_lock-------------------\n");*/
  /*if (SemHandleWait(i_sem) == SYNC_FAIL) {
    dbprintf('f', "DfsInodeAllocateVirtualBlock (%d): could not release the inode sem\n", GetCurrentPid());
    return DFS_FAIL;
  }
  dbprintf('f', "-------------------DfsInodeAllocateVirtualBlock RELEASED f_sem-------------------\n");*/
  
  return fs_block;
}

//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

int DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) {
  dfs_block v_block;
  v_table   v_t;
  if (!sb.valid) {
    dbprintf('f', "DfsInodeFilesize (%d): file system is not valid\n", GetCurrentPid());
    return DFS_FAIL;
  }

  if (!inodes[handle].inuse) {
    dbprintf('f', "DfsInodeFilesize (%d): inode trying to access is not in use\n", GetCurrentPid());
    return DFS_FAIL;
  }

  // return fs blocknum in direct addr table
  if (virtual_blocknum < NUM_ADDR_BLOCK)
    return inodes[handle].directAddr[virtual_blocknum];

  if ((DfsReadBlock(inodes[handle].indirectAddr, &v_block)) == DFS_FAIL) {
    dbprintf('f', "DfsInodeTranslateVirtualToFilesys (%d): Failed to read block\n", GetCurrentPid());
    return DFS_FAIL;
  }
  bcopy((char *)v_block.data, (char *)(&v_t), sb.fsBlocksize);

  return v_t.addr[virtual_blocknum - NUM_ADDR_BLOCK]; // each entry is uint32??
}

void InodeTest() {
  uint32 f_handle;
  uint32 block_num;
  uint32 i;
  char *write_message, *read_message, *read_message2;

  if ((f_handle = DfsInodeOpen("test.txt")) == DFS_FAIL) {
    printf("RunOSTests: failed to open new inode 1\n");
    GracefulExit();
  }
  
  printf("inode handle %d\n", f_handle);

  // Allocate into indirect address space
  for (i = 0; i < 138; i++) { // TODO breaks from 139 onwards
    if ((block_num = DfsInodeAllocateVirtualBlock(f_handle, i)) == DFS_FAIL) {
      printf("RunOSTests: failed to allocate virtual blocks\n");
      GracefulExit();
    }
    printf("Allocated block: %d\n", block_num);
  }

  write_message = "new inode write\0";
  printf("%d\n", DfsInodeWriteBytes(f_handle, (void*)write_message, 40, dstrlen(write_message)));
  printf("%d\n", DfsInodeReadBytes(f_handle, (void*)read_message, 40, dstrlen("new inode write")));
  printf("read message: %s\n\n", (char*)read_message);

  write_message = "same block\0";
  DfsInodeWriteBytes(f_handle, (void*)write_message, 123, dstrlen(write_message));
  DfsInodeReadBytes(f_handle, (void*)read_message2, 123, dstrlen("same block"));
  printf("read message: %s\n\n", read_message2);

  write_message = "across blocks\0";
  DfsInodeWriteBytes(f_handle, (void*)write_message, 1020, dstrlen(write_message));
  DfsInodeReadBytes(f_handle, (void*)read_message, 1020, dstrlen("across blocks"));
  printf("read message: %s\n\n", read_message);

  write_message = "indirect space\0";
  DfsInodeWriteBytes(f_handle, (void*)write_message, 12345, dstrlen(write_message));
  DfsInodeReadBytes(f_handle, (void*)read_message, 12345, dstrlen("indirect space"));
  printf("read message: %s\n\n", read_message);

  printf("handle: %d\n", f_handle);

  /*if ((f_handle = DfsInodeDelete(f_handle)) == DFS_FAIL) {
    printf("RunOSTests: failed to delete inode\n");
    GracefulExit();
  }*/
  
  /*if ((f_handle = DfsInodeOpen("test1.txt")) == DFS_FAIL) {
    printf("RunOSTests: failed to open new inode 2\n");
    GracefulExit();
  }

  if ((f_handle = DfsInodeDelete(f_handle)) == DFS_FAIL) {
    printf("RunOSTests: failed to delete inode\n");
    GracefulExit();
  }*/
}
