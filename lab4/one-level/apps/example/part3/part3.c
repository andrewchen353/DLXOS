#include "usertraps.h"
#include "misc.h"

#define NUM_USER_STACK 10

void growStack(int i)
{
  char random_buff[4096];
  int page_num;

  Printf("\nAfter allocating 4KB in character buffers in %d calls\n", i);
  Printf("New page address: %d\n", &page_num);
  Printf("Page number: %d\n", (int)(&page_num) >> 12);

  if (i < NUM_USER_STACK)
    growStack(++i);
  else
    return;
}

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  growStack(0);

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }
}
