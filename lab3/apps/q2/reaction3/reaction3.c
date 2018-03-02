#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char* argv[])
{
  sem_t s_procs_completed;  // Semaphore to signal the original process that we're done
  mbox_t h2so4_handle;
  mbox_t so2_handle;
  mbox_t h2_handle;
  mbox_t o2_handle;
  void* msg;

  if (argc != 6) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_page_mapped_semaphore> <handle to h2o mailbox>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  h2_handle = dstrtol(argv[2], NULL, 10); // The "10" means base 10
  o2_handle = dstrtol(argv[3], NULL, 10); // The "10" means base 10
  so2_handle = dstrtol(argv[4], NULL, 10); // The "10" means base 10
  h2so4_handle = dstrtol(argv[5], NULL, 10); // The "10" means base 10

  if (mbox_open(h2_handle) != MBOX_SUCCESS) {
    Printf("Could not open h2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(o2_handle) != MBOX_SUCCESS) {
    Printf("Could not open o2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(so2_handle) != MBOX_SUCCESS) {
    Printf("Could not open so2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(h2so4_handle) != MBOX_SUCCESS) {
    Printf("Could not open h2so4 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // recv h2o molecules
  if(mbox_recv(h2_handle, 3, msg) != MBOX_SUCCESS) {
    Printf("Bad h2 mailbox recv (%d) in ", h2_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  //Printf("**************h2: %d\n", h2_handle);
  //Printf("----------R3 h2 Message Received: %s\n", msg);
  Printf("Process %d received an H2 molecule\n", getpid());
  if(mbox_recv(o2_handle, 3, msg) != MBOX_SUCCESS) {
    Printf("Bad o2 mailbox recv (%d) in ", o2_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  //Printf("**************o2: %d\n", o2_handle);
  //Printf("----------R3 o2 Message Received: %s\n", msg);
  Printf("Process %d received an O2 molecule\n", getpid());
  if(mbox_recv(so2_handle, 4, msg) != MBOX_SUCCESS) {
    Printf("Bad so2 mailbox recv (%d) in ", so2_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  //Printf("**************so2: %d\n", so2_handle);
  //Printf("----------R3 so2 Message Received: %s\n", msg);
  Printf("Process %d received an SO2 molecule\n", getpid());

  // send h2o molecules
  if(mbox_send(h2so4_handle, 6, (void*)"h2so4") != MBOX_SUCCESS) {
    Printf("Bad h2so4 mailbox send (%d) in ", h2so4_handle); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  //Printf("**************h2so4: %d\n", h2so4_handle);
  Printf("Process %d sent 1 H2SO4\n", getpid());

  if (mbox_close(h2_handle) != MBOX_SUCCESS) {
    Printf("Could not close h2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(o2_handle) != MBOX_SUCCESS) {
    Printf("Could not close o2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(so2_handle) != MBOX_SUCCESS) {
    Printf("Could not close so2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(h2so4_handle) != MBOX_SUCCESS) {
    Printf("Could not close h2so4 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("Reaction 3: PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
