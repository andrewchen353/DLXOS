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
  so4_handle = dstrtol(argv[2], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  if (mbox_open(so4_handle) != MBOX_SUCCESS)
  {
    Printf("ERROR: failed to open so4 %d mailbox\n", so4_handle);
    Exit();
  }

  // inject so4 molecules
  if(mbox_send(so4_handle, 3, "so4") != MBOX_SUCCESS) {
    Printf("Bad so4 mailbox send (%d) in ", so4_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  Printf("Process %d injected an H2O molecule\n", getpid());

  if (mbox_close(so4_handle) != MBOX_SUCCESS)
  {
    Printf("ERROR: failed to close so4 %d mailbox\n", so4_handle);
    Exit();
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("Producer (H2O): PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}