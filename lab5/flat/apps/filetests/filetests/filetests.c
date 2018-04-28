#include "usertraps.h"
#include "files_shared.h"

void main (int argc, char *argv[])
{
  int handle;
  char *write_message;

  if ((handle = file_open("test.txt", "w")) == FILE_FAIL) {
    Printf("Failed to open file \"test.txt\"\n");
    return;
  } else {
    Printf("File successfully opened\n");
  }
  write_message = "Checking to see if the write works";
    
  
}
