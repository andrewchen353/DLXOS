#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

// STUDENT: put your file-level functions here

int FileOpen(char* filename, char* mode) {
  return FILE_SUCCESS;
}

int FileClose(int handle) {
  return FILE_SUCCESS;
}

int FileRead(int handle, void* mem, int num_bytes) {
  return FILE_SUCCESS;
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

