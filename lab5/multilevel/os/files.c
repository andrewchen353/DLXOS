#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

static file_descriptor files[FILE_MAX_OPEN_FILES];
static lock_t file_lock;
extern PCB* currPCB;

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
  dbprintf('f', "Finished with file init\n");
  return;
}

void FilenameSplit(char *path, char **store, int *num) {
  int i, j, k, curr;

  i = 0;
  k = 0;
  curr = 0;
  while (path[i] != '\0') {
    if (path[i] == '/') {
      for (j = k; j < i; j++)
        store[curr][j] = path[j];
      store[curr][j] = '\0';
      curr++;
      k = i;
    }
  }
  num = curr;
}

int FileOpen(char* filename, char* mode) {
  int i;
  int *numPaths;
  char **paths;

  dbprintf('f', "FileOpen (%d), Entering function\n", GetCurrentPid());
  
  FilenameSplit(filename, paths, numPaths);  

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
      if (mode[0] == 'r')
        if (mode[1] == 'w')
          files[i].mode = FILE_MODE_RW;
        else
          files[i].mode = FILE_MODE_R;
      else if (mode[0] == 'w')
        files[i].mode = FILE_MODE_W;  
      else {
        dbprintf('f', "FileOpen (%d), Mode given doesn't exist\n", GetCurrentPid());
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

}

int FileRead(int handle, void* mem, int num_bytes) {

}

int FileWrite(int handle, void* mem, int num_bytes) {

}

int FileDelete(char* path) {

}

int MkDir(char* path, int permissions) {

}

int RmDir(char* path) {

}
