#include "usertraps.h"
#include "misc.h"
#include "files_shared.h"

void main (int argc, char *argv[])
{
  int handle;
  char *write_message;

  //handle = file_open("test.txt", "rw");
  if ((handle = file_open("test.txt", "rw")) == FILE_FAIL) {
    Printf("Failed to open file \"test.txt\"\n");
    return;
  } else {
    Printf("File successfully opened\n");
  }
  //Printf("handle %d\n", handle);
  write_message = "Checking to see if the write works";
    
  
}
