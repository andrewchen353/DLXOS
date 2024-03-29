#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char* argv[])
{
  sem_t s_procs_completed;  // Semaphore to signal the original process that we're done
  mbox_t h2o_handle;

  if (argc != 3) { 
    Printf("Usage: %d ", argc); Printf(argv[0]); Printf(" <handle_to_page_mapped_semaphore> <handle to h2o mailbox>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  h2o_handle = dstrtol(argv[2], NULL, 10); // The "10" means base 10

  if (mbox_open(h2o_handle) != MBOX_SUCCESS) {
    Printf("Could not open h2o mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // inject h2o molecules
  if(mbox_send(h2o_handle, 4, (void*)"h2o") != MBOX_SUCCESS) {
    Printf("Bad h2o mailbox send (%d) in ", h2o_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  Printf("Process %d injected an H2O molecule\n", getpid());

  if (mbox_close(h2o_handle) != MBOX_SUCCESS) {
    Printf("Could not close h2o mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("Producer (H2O): PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
