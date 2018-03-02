#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char* argv[])
{
  sem_t s_procs_completed;  // Semaphore to signal the original process that we're done
  mbox_t h2o_handle;
  mbox_t h2_handle;
  mbox_t o2_handle;
  void* msg;

  if (argc != 5) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_page_mapped_semaphore> <handle to h2o mailbox>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  h2o_handle = dstrtol(argv[2], NULL, 10); // The "10" means base 10
  h2_handle = dstrtol(argv[3], NULL, 10); // The "10" means base 10
  o2_handle = dstrtol(argv[4], NULL, 10); // The "10" means base 10

  if (mbox_open(h2o_handle) != MBOX_SUCCESS) {
    Printf("Could not open h2o mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(h2_handle) != MBOX_SUCCESS) {
    Printf("Could not open h2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(o2_handle) != MBOX_SUCCESS) {
    Printf("Could not open o2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // recv h2o molecules
  if(mbox_recv(h2o_handle, 4, msg) != MBOX_SUCCESS) {
    Printf("Bad h2o mailbox recv (%d) in ", h2o_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  Printf("Process %d received an H2O molecule 1\n", getpid());
  //Printf("**************h2o: %d\n", h2o_handle);
  if(mbox_recv(h2o_handle, 4, msg) != MBOX_SUCCESS) {
    Printf("Bad h2o mailbox recv (%d) in ", h2o_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  //Printf("**************h2o: %d\n", h2o_handle);
  //Printf("------------R1 Message Received: %s\n", (char*)msg);
  Printf("Process %d received an H2O molecule 2\n", getpid());

  // send h2o molecules
  if(mbox_send(h2_handle, 3, (void*)"h2") != MBOX_SUCCESS) {
    Printf("Bad h2 mailbox send (%d) in ", h2_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  //Printf("**************h2: %d\n", h2_handle);
  if(mbox_send(h2_handle, 3, (void*)"h2") != MBOX_SUCCESS) {
    Printf("Bad h2 mailbox send (%d) in ", h2_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  //Printf("**************h2: %d\n", h2_handle);
  if(mbox_send(o2_handle, 3, (void*)"o2") != MBOX_SUCCESS) {
    Printf("Bad o2 mailbox send (%d) in ", o2_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  //Printf("**************o2: %d\n", o2_handle);
  Printf("Process %d sent 2 H2 and 1 O2\n", getpid());

  if (mbox_close(h2o_handle) != MBOX_SUCCESS) {
    Printf("Could not close h2o mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(h2_handle) != MBOX_SUCCESS) {
    Printf("Could not close h2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(o2_handle) != MBOX_SUCCESS) {
    Printf("Could not close o2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("Reaction 1: PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
