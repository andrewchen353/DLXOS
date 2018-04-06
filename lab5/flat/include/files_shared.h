#ifndef __FILES_SHARED__
#define __FILES_SHARED__

#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3

#define FILE_MAX_FILENAME_LENGTH 76

#define FILE_MAX_READWRITE_BYTES 4096

typedef struct file_descriptor {
  // STUDENT: put file descriptor info here
  uint32     inuse;
  char       filename[FILE_MAX_FILENAME_LENGTH];
  //TODO check types below
  dfs_inode* inode;
  uint32     eof;
  uint32     mode;
  uint32     currentPosition;
  uint32     processID;
} file_descriptor;

#define FILE_FAIL -1
#define FILE_EOF -1
#define FILE_SUCCESS 1

#endif
