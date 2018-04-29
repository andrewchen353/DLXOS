#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "traps.h"
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
  dbprintf('f', "FileModuleInit(%d): INIT\n", GetCurrentPid());
  for (i = 0; i < FILE_MAX_OPEN_FILES; i++)
    files[i].inuse = 0;
  if ((file_lock = LockCreate()) == SYNC_FAIL) {
    dbprintf('f', "FileModuleInit (%d): Failed to create file_lock\n", GetCurrentPid());
    GracefulExit();
  }
  dbprintf('f', "Finished with file init------------------------------------------------------------------------------------------------------\n");
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
    if (LockHandleAcquire(file_lock) == SYNC_FAIL) {
      dbprintf('f', "FileOpen(%d): Failed to acquire file_lock\n", GetCurrentPid());
      return FILE_FAIL;
    }
    if (!files[i].inuse) {
      files[i].inuse = 1;
      files[i].processID = GetCurrentPid();
      if (!dstrncmp("r", mode, dstrlen(mode)))
        files[i].mode = FILE_MODE_R;
      else if (!dstrncmp("w", mode, dstrlen(mode)))
        files[i].mode = FILE_MODE_W;
      else if (!dstrncmp("rw", mode, dstrlen(mode)))
        files[i].mode = FILE_MODE_RW;
      else {
        dbprintf('f', "FileOpen (%d), Mode given doesnt exist\n", GetCurrentPid());
        files[i].inuse = 0;
        return FILE_FAIL;
      }

      files[i].currentPosition = 0;
      files[i].eof = 0;
      files[i].inode = DfsInodeOpen(filename);
      dstrcpy(files[i].filename, filename);
      break;
    }
    if (LockHandleRelease(file_lock) == SYNC_FAIL) {
      dbprintf('f', "FileOpen(%d): Failed to release file_lock\n", GetCurrentPid());
      return FILE_FAIL;
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
  //int i;

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
 
  if (files[handle].mode == FILE_MODE_W) {
    dbprintf('f', "FileRead (%d), File mode not set to read\n", GetCurrentPid());
    return FILE_FAIL;
  }

  if (files[handle].eof) {
    dbprintf('f', "FileRead (%d), Cannot read past end of file\n", GetCurrentPid());
    return FILE_FAIL;
  }

  if (files[handle].currentPosition + num_bytes == (DfsInodeFilesize(files[handle].inode) - 1)) {
    files[handle].eof = 1;
  } else if (files[handle].currentPosition + num_bytes >= DfsInodeFilesize(files[handle].inode)) {
    num_bytes = DfsInodeFilesize(files[handle].inode) - files[handle].currentPosition - 1;
    files[handle].eof = 1; 
  }
 
  if ((bytes_read = DfsInodeReadBytes(files[handle].inode, mem, files[handle].currentPosition, num_bytes)) == DFS_FAIL) {
    dbprintf('f', "FileRead (%d), Nothing to read from file descriptor\n", GetCurrentPid());
    return FILE_FAIL;
  }

  files[handle].currentPosition += bytes_read;

  return bytes_read;
}

int FileWrite(int handle, void* mem, int num_bytes) {
  uint32 bytes_written;
  
  dbprintf('f', "FileWrite (%d), Entering function\n", GetCurrentPid());
  
  if (handle >= FILE_MAX_OPEN_FILES || handle < 0) {
    dbprintf('f', "FileWrite (%d), handle too big man\n", GetCurrentPid());
    return FILE_FAIL;
  } else if (!files[handle].inuse) {
    dbprintf('f', "FileWrite (%d), Cannot write to an unopened file\n", GetCurrentPid());
    return FILE_FAIL;
  } else if (num_bytes > FILE_MAX_READWRITE_BYTES) {
    dbprintf('f', "FileWrite (%d), Cannot write more than %d bytes at a time\n", GetCurrentPid(), FILE_MAX_READWRITE_BYTES);
    return FILE_FAIL;
  }

  if (files[handle].mode == FILE_MODE_R) {
    dbprintf('f', "FileWrite (%d), File mode not set to write\n", GetCurrentPid());
    return FILE_FAIL;
  }

  if (files[handle].eof) {
    dbprintf('f', "FileWrite (%d), Cannot write past end of file\n", GetCurrentPid());
    return FILE_FAIL;
  }

  if ((bytes_written = DfsInodeWriteBytes(files[handle].inode, mem, files[handle].currentPosition, num_bytes)) == DFS_FAIL) {
    dbprintf('f', "FileRead (%d), Nothing written from file descriptor\n", GetCurrentPid());
    return FILE_FAIL;
  }

  files[handle].currentPosition += bytes_written;

  dbprintf('f', "FileWrite (%d), Leaving function\n", GetCurrentPid());

  return bytes_written;
}

int FileSeek(int handle, int num_bytes, int from_where) {
  dbprintf('f', "FileSeek (%d), Entering function\n", GetCurrentPid());
  
  if (handle >= FILE_MAX_OPEN_FILES || handle < 0) {
    dbprintf('f', "FileSeek (%d), handle too big man\n", GetCurrentPid());
    return FILE_FAIL;
  } else if (!files[handle].inuse) {
    dbprintf('f', "FileSeek (%d), Cannot write to an unopened file\n", GetCurrentPid());
    return FILE_FAIL;
  } else if (num_bytes > FILE_MAX_READWRITE_BYTES) {
    dbprintf('f', "FileSeek (%d), Cannot write more than %d bytes at a time\n", GetCurrentPid(), FILE_MAX_READWRITE_BYTES);
    return FILE_FAIL;
  }

  if (from_where == FILE_SEEK_SET) {
    files[handle].currentPosition = num_bytes;
  } else if (from_where == FILE_SEEK_END) {
    files[handle].currentPosition = DfsInodeFilesize(files[handle].inode) - num_bytes;
  } else if (from_where == FILE_SEEK_CUR) {
    files[handle].currentPosition += num_bytes;
  } else {
    dbprintf('f', "FileSeek (%d), Incorrect from_where\n", GetCurrentPid());
    return FILE_FAIL;
  }

  files[handle].eof = 0;
  
  dbprintf('f', "FileSeek (%d), Leaving function\n", GetCurrentPid());
  
  return FILE_SUCCESS;
}

int FileDelete(char* filename) {
  int i;

  dbprintf('f', "FileDelete (%d), Entering function\n", GetCurrentPid());
  
  for(i = 0; i < FILE_MAX_OPEN_FILES; i++) {
    if(!dstrncmp(files[i].filename, filename, dstrlen(filename)))
      break;
  }

  if (i == FILE_MAX_OPEN_FILES) {
    dbprintf('f', "FileDelete (%d), File to delete not found\n", GetCurrentPid());
    return FILE_FAIL;
  }

  if(DfsInodeDelete(files[i].inode) == DFS_FAIL) {
    dbprintf('f', "FileDelete (%d), Cannot not delete inode\n", GetCurrentPid());
    return FILE_FAIL;
  }

  files[i].inuse = 0;
  *(files[i].filename) = NULL;

  dbprintf('f', "FileDelete (%d), Leaving function\n", GetCurrentPid());
  
  return FILE_SUCCESS;
}

uint32 FileDescFilenameExists(char *filename) {
  int i;
  for(i = 0; i < FILE_MAX_OPEN_FILES; i++) {
    if(!dstrncmp(files[i].filename, filename, dstrlen(filename)))
      break;
  }
  return i;
}

