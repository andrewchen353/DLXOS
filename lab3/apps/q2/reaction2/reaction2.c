#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char* argv[])
{
  sem_t s_procs_completed;  // Semaphore to signal the original process that we're done
  mbox_t so4_handle;
  mbox_t so2_handle;
  mbox_t o2_handle;
  void* msg;

  if (argc != 5) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_page_mapped_semaphore> <handle to so4 mailbox>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  so4_handle = dstrtol(argv[2], NULL, 10); // The "10" means base 10
  so2_handle = dstrtol(argv[3], NULL, 10); // The "10" means base 10
  o2_handle = dstrtol(argv[4], NULL, 10); // The "10" means base 10

  if (mbox_open(so4_handle) != MBOX_SUCCESS)
  {
    Printf("ERROR: failed to open so4 %d mailbox\n", so4_handle);
    Exit();
  }
  if (mbox_open(so2_handle) != MBOX_SUCCESS)
  {
    Printf("ERROR: failed to open so2 %d mailbox\n",so2_handle);
    Exit();
  }
  if (mbox_open(o2_handle) != MBOX_SUCCESS)
  {
    Printf("ERROR: failed to open o2 %d mailbox\n", o2_handle);
    Exit();
  }

  // recv so4 molecules
  if(mbox_recv(so4_handle, 24, msg) != MBOX_SUCCESS) {
    Printf("Bad so4 mailbox recv (%d) in ", so4_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  Printf("Process %d received an SO4 molecule\n", getpid());

  // send so4 molecules
  if(mbox_send(so2_handle, 3, (void*)"so2") != MBOX_SUCCESS) {
    Printf("Bad so2 mailbox send (%d) in ", so2_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  if(mbox_send(o2_handle, 2, (void*)"o2") != MBOX_SUCCESS) {
    Printf("Bad o2 mailbox send (%d) in ", o2_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  Printf("Process %d sent 1 SO2 and 1 O2\n", getpid());

  if (mbox_close(so4_handle) != MBOX_SUCCESS)
  {
    Printf("ERROR: failed to close so4 %d mailbox\n", so4_handle);
    Exit();
  }
  if (mbox_close(so2_handle) != MBOX_SUCCESS)
  {
    Printf("ERROR: failed to close so2 %d mailbox\n", so2_handle);
    Exit();
  }
  if (mbox_close(o2_handle) != MBOX_SUCCESS)
  {
    Printf("ERROR: failed to close o2 %d mailbox\n", o2_handle);
    Exit();
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("Reaction 2: PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
