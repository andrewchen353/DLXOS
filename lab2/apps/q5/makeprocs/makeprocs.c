#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int numh2o = 0;                     // Used to store number of h2o molecules
  int numso4 = 0;                     // Used to store number of so4 molecules
  int numr1 = 0;                     // Used to store number of h2o molecules
  int numr2 = 0;                     // Used to store number of h2o molecules
  int numr3 = 0;                     // Used to store number of h2o molecules
  uint32 h_mem;                       // Used to hold handle to shared memory page
  sem_t s_procs_completed;            // Semaphores for completion of all processes
  chem_semaphore *cs;                  // molecule/reaction semaphores
  char s_procs_completed_str[10];     // Used as command-line argument to pass mem_handle to new processes
  char h_mem_str[10];                 // Used as command-line argument to pass mem_handle to new processes 
  char h2o_str[10];                 // Used as command-line argument to pass mem_handle to new processes 
  char so4_str[10];                 // Used as command-line argument to pass mem_handle to new processes 
  char r1_str[10];                 // Used as command-line argument to pass mem_handle to new processes 
  char r2_str[10];                 // Used as command-line argument to pass mem_handle to new processes 
  char r3_str[10];                 // Used as command-line argument to pass mem_handle to new processes 

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  numh2o = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("Creating %d processes\n", numh2o);

  numso4 = dstrtol(argv[2], NULL, 10); // the "10" means base 10
  Printf("Creating %d processes\n", numso4);

  if ((h_mem = shmget()) == 0) {
    Printf("ERROR: could not allocate shared memory page in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  
  if ((cs = (chem_semaphore *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
    Exit();
  } 

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  if ((s_procs_completed = sem_create(-(5 - 1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((cs->h2o = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((cs->so4 = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((cs->h2 = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((cs->o2 = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((cs->so4 = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((cs->h2so4 = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  
  //Calculate number of each molecules
  numr1 = numh2o / 2;
  numr2 = numso4;
  if ((2*numr1) < (numr1 + numr2) && (2*numr1) < (numr2))
    numr3 = (2*numr1);
  else if ((numr1 + numr2) < (2*numr1) && (numr1 + numr2) < (numr2))
    numr3 = (numr1 + numr2);
  else
    numr3 = numr2;

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(h_mem, h_mem_str);
  ditoa(numh2o, h2o_str);
  ditoa(numso4, so4_str);
  ditoa(numr1, r1_str);
  ditoa(numr2, r2_str);
  ditoa(numr3, r3_str);

  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  process_create(H2O_TO_RUN, s_procs_completed_str, h_mem_str, h2o_str, NULL); // h2o
  process_create(SO4_TO_RUN, s_procs_completed_str, h_mem_str, so4_str, NULL); // so4
  process_create(R1_TO_RUN, s_procs_completed_str, h_mem_str, r1_str, NULL); // reaction 1
  process_create(R2_TO_RUN, s_procs_completed_str, h_mem_str, r2_str, NULL); // reaction 2
  process_create(R3_TO_RUN, s_procs_completed_str, h_mem_str, r3_str, NULL); // reaction 3
  Printf("Process %d created\n", 5);

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad condition s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");

  return;
}
