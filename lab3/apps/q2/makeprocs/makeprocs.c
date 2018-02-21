#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int numh2o = 0;                   // Used to store number of h2o molecules
  int numso4 = 0;                   // Used to store number of so4 molecules
  int numr1 = 0;                    // Used to store number of h2o molecules
  int numr2 = 0;                    // Used to store number of h2o molecules
  int numr3 = 0;                    // Used to store number of h2o molecules
  sem_t s_procs_completed;          // Semaphores for completion of all processes
  char s_procs_completed_str[10];   // Used as command-line argument to pass mem_handle to new processes
  mbox_t h2o_handle;
  mbox_t so4_handle;
  mbox_t so2_handle;
  mbox_t h2_handle;
  mbox_t o2_handle;
  mbox_t h2so4_handle;
  char h2o_str[10];
  char so4_str[10];
  char h2_str[10];
  char so2_str[10];
  char o2_str[10];
  char h2so4_str[10];

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of h2o molecules to create> <number of so4 molecules to create\n");
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  numh2o = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("Creating %d processes\n", numh2o);

  numso4 = dstrtol(argv[2], NULL, 10); // the "10" means base 10
  Printf("Creating %d processes\n", numso4);

  //Calculate number of each molecules
  numr1 = numh2o / 2;
  numr2 = numso4;
  if ((2*numr1) < (numr1 + numr2) && (2*numr1) < (numr2))
    numr3 = (2*numr1);
  else if ((numr1 + numr2) < (2*numr1) && (numr1 + numr2) < (numr2))
    numr3 = (numr1 + numr2);
  else
    numr3 = numr2;

  // numprocs = numh2o/2 + numso4
  if ((s_procs_completed = sem_create(-(numr1 + numr2 + numr3 + numh2o + numso4 - 1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // create mailbox
  h2o_handle = mbox_create();
  so4_handle = mbox_create();
  h2_handle = mbox_create();
  so2_handle = mbox_create();
  o2_handle = mbox_create();
  h2so4_handle = mbox_create();

  Printf("%d %d %d %d %d %d \n", h2o_handle, so4_handle, h2_handle, so2_handle, o2_handle, h2so4_handle);


  if (mbox_open(h2o_handle) != MBOX_SUCCESS) {
    Printf("Could not open h2o mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(so4_handle) != MBOX_SUCCESS) {
    Printf("Could not open so4 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(h2_handle) != MBOX_SUCCESS) {
    Printf("Could not open h2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(so2_handle) != MBOX_SUCCESS) {
    Printf("Could not open so2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(o2_handle) != MBOX_SUCCESS) {
    Printf("Could not open o2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(h2so4_handle) != MBOX_SUCCESS) {
    Printf("Could not open h2so4 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(h2o_handle, h2o_str);
  ditoa(so4_handle, so4_str);
  ditoa(h2_handle, h2_str);
  ditoa(so2_handle, so2_str);
  ditoa(o2_handle, o2_str);
  ditoa(h2so4_handle, h2so4_str);
  //Printf("%s %s %s %s %s %s \n", h2o_str, so4_str, h2_str, so2_str, o2_str, h2so4_str);
  Printf("%s\n", h2o_str);
  Printf("%s\n", so4_str);
  Printf("%s\n", h2_str);
  Printf("%s\n", so2_str);
  Printf("%s\n", o2_str);
  Printf("%s\n", h2so4_str);
  
  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending

  while (numh2o > 0) {
    process_create(H2O_TO_RUN, 0, 0, s_procs_completed_str, h2o_str, NULL);
    numh2o--;
    Printf("Process h2o inject created\n");
  }
  while (numso4 > 0) {
    process_create(SO4_TO_RUN, 0, 0, s_procs_completed_str, so4_str, NULL);
    numso4--;
    Printf("Process so4 inject created\n");
  }
  while (numr1 > 0) {
    process_create(R1_TO_RUN, 0, 0, s_procs_completed_str, h2o_str, h2_str, o2_str, NULL);
    numr1--;
    Printf("Process reaction 1 created\n");
  }
  while (numr2 > 0) {
    process_create(R2_TO_RUN, 0, 0, s_procs_completed_str, so4_str, so2_str, o2_str, NULL);
    numr2--;
    Printf("Process reaction 2 created\n");
  }
  while (numr3 > 0) {
    process_create(R3_TO_RUN, 0, 0, s_procs_completed_str, h2_str, o2_str, so2_str, h2so4_str, NULL);
    numr3--;
    Printf("Process reaction 3 created\n");
  }

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad condition s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // close mailboxes
  if (mbox_close(h2o_handle) != MBOX_SUCCESS) {
    Printf("Could not close h2o mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(so4_handle) != MBOX_SUCCESS) {
    Printf("Could not close so4 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(h2_handle) != MBOX_SUCCESS) {
    Printf("Could not close h2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(so2_handle) != MBOX_SUCCESS) {
    Printf("Could not close so2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(o2_handle) != MBOX_SUCCESS) {
    Printf("Could not close o2 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(h2so4_handle) != MBOX_SUCCESS) {
    Printf("Could not close h2so4 mailbox in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");

  return;
}
