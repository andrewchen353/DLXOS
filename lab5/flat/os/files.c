#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.
static file_descriptor files[FILE_MAX_OPEN_FILES];
static lock_t file_lock;

// STUDENT: put your file-level functions here

void FileModuleInit() {
  int i;
  for (i = 0; i < FILE_MAX_OPEN_FILES; i++)
    files[i].inuse = 0;
  return;
}

int FileOpen(char* filename, char* mode) {
  int i;

  dbprintf('f', "FileOpen (%d), Entering function\n", GetCurrentPid());
  
  for (i = 0; i < FILE_MAX_OPEN_FILES; i++) {
    if (files[i].inuse && !dstrncmp(files[i].filename, filename, dstrlen(filename))) {
      dbprintf('f', "FileOpen (%d), filename trying to open has already been opened\n", GetCurrentPid());
      return FILE_FAIL;
    }
  }

  for (i = 0; i < FILE_MAX_OPEN_FILES; i++) {
    if (!files[i].inuse) {
      files[i].inuse = 1;
      files[i].processID = GetCurrentPid();
      files[i].mode = (uint32)mode;
      files[i].currentPosition = 0;
      files[i].eof = 0;
      files[i].inode = DfsInodeOpen(filename);
      dstrcpy(files[i].filename, filename);
      break;
    }
  }

  if (i == FILE_MAX_OPEN_FILES) {
    dbprintf('f', "FileOpen (%d), no more files can be open\n", GetCurrentPid());
    return FILE_FAIL;
  }

  dbprintf('f', "FileOpen (%d), Leaving function\n", GetCurrentPid());
  
  return i;
}

int FileClose(int handle) {
  int i;

  dbprintf('f', "FileClose (%d), Entering function\n", GetCurrentPid());

  if (handle >= FILE_MAX_OPEN_FILES || handle < 0) {
    dbprintf('f', "FileClose (%d), handle too big man\n", GetCurrentPid());
    return FILE_FAIL;
  } else if (!files[handle].inuse) {
    dbprintf('f', "FileClose (%d), Cannot close a file that is not inuse\n", GetCurrentPid());
    return FILE_FAIL;
  }

  files[handle].inuse = 0;

  dbprintf('f', "FileClose (%d), Leaving function\n", GetCurrentPid());

  return FILE_SUCCESS;
}

int FileRead(int handle, void* mem, int num_bytes) {
  uint32 bytes_read;
  
  dbprintf('f', "FileRead (%d), Entering function\n", GetCurrentPid());

  if (handle >= FILE_MAX_OPEN_FILES || handle < 0) {
    dbprintf('f', "FileRead (%d), handle too big man\n", GetCurrentPid());
    return FILE_FAIL;
  } else if (!files[handle].inuse) {
    dbprintf('f', "FileRead (%d), Cannot read from an unopened file\n", GetCurrentPid());
    return FILE_FAIL;
  } else if (num_bytes > FILE_MAX_READWRITE_BYTES) {
    dbprintf('f', "FileRead (%d), Cannot read more than %d bytes at a time\n", GetCurrentPid(), FILE_MAX_READWRITE_BYTES);
    return FILE_FAIL;
  }
  
  if ((bytes_read = DfsInodeReadBytes(files[handle].inode, mem, files[handle].currentPosition, num_bytes)) == DFS_FAIL) {
    dbprintf('f', "FileRead (%d), Nothing to read from file descriptor\n", GetCurrentPid());
    return FILE_FAIL;
  }

  return bytes_read;
}

int FileWrite(int handle, void* mem, int num_bytes) {
  return FILE_SUCCESS;
}

int FileSeek(int handle, int num_bytes, int from_where) {
  return FILE_SUCCESS;
}

int FileDelete(char* filename) {
  return FILE_SUCCESS;
}

uint32 FileDescFilenameExists(char *filename) {
  return FILE_SUCCESS;
}

