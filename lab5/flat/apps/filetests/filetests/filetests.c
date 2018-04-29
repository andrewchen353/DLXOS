#include "usertraps.h"
#include "misc.h"
#include "files_shared.h"

#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3

void main (int argc, char *argv[])
{
  int handle;
  char *write_message, *read_message, *message;


  Printf("\nTEST1********************************************\n");
  //handle = file_open("test.txt", "rw");
  if ((handle = file_open("test.txt", "rw")) == FILE_FAIL) {
    Printf("Failed to open file \"test.txt\"\n");
    return;
  } else {
    Printf("File successfully opened\n");
  }
  //Printf("handle %d\n", handle);
  write_message = "Checking to see if the write works";
  
  Printf("Writing \"Checking to see if the write works\"\n");
  file_write(handle, (void*)write_message, dstrlen(write_message));
  Printf("Finished writing\n");

  file_seek(handle, 0, FILE_SEEK_SET);

  Printf("Reading...\n");
  file_read(handle, (void*)read_message, dstrlen(write_message));
  Printf("%s\n", read_message);

  file_close(handle);
  Printf("Finished closing\n");

  Printf("\nTEST2******************************************\n");
  if ((handle = file_open("test.txt", "rw")) == FILE_FAIL) {
    Printf("Failed to open file \"test.txt\"\n");
    return;
  } else {
    Printf("File successfully opened\n");
  }

  Printf("Reading...\n");
  file_read(handle, (void*)read_message, dstrlen(write_message));
  Printf("%s\n", read_message);

  write_message = NULL;
  write_message = "something is working";
  Printf("Writing...\n%s   len %d\n", write_message, dstrlen(write_message));
  file_write(handle, (void*)write_message, dstrlen(write_message));
  
  file_seek(handle, -dstrlen(write_message), FILE_SEEK_CUR);
  read_message = NULL;
  Printf("Reading...\n");
  file_read(handle, (void*) read_message, dstrlen(read_message));
  Printf("Read: %s\n", read_message);

  file_close(handle);
  Printf("Finished closing\n");

  file_delete("test.txt");
  Printf("Finished deleting\n");

  Printf("\nTEST3*********************************************\n");
  if ((handle = file_open("test.txt", "rw")) == FILE_FAIL) {
    Printf("Failed to open file \"test.txt\"\n");
    return;
  } else {
    Printf("File successfully opened\n");
  }

  if (file_read(handle, (void*)read_message, dstrlen(read_message)) == FILE_FAIL)
    Printf("Correctly not able to read from empty file\n");
  else
    Printf("We read something :( %s\n", read_message);

  file_close(handle);
  Printf("Finished closing\n");

  file_delete("test.txt");
  Printf("Finished deleting\n");
}
