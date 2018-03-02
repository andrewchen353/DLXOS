#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char* argv[])
{
  sem_t s_procs_completed;  // Semaphore to signal the original process that we're done
  mbox_t so4_handle;

  if (argc != 3) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_page_mapped_semaphore> <handle to so4 mailbox>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  so4_handle = dstrtol(argv[2], NULL, 10); // The "10" means base 10

  if (mbox_open(so4_handle) != MBOX_SUCCESS) {
    Printf("Could not open so4 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // inject so4 molecules
  if(mbox_send(so4_handle, 4, (void*)"so4") != MBOX_SUCCESS) {
    Printf("Bad so4 mailbox send (%d) in ", so4_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  Printf("Process %d injected an SO4 molecule\n", getpid());

  if (mbox_close(so4_handle) != MBOX_SUCCESS) {
    Printf("Could not close so4 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("Producer (SO4): PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
