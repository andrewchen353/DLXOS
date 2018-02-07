#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int numh20 = 0;                     // Used to store number of h2o molecules
  int numso4 = 0;                     // Used to store number of so4 molecules
  int numh2 = 0;                     // Used to store number of h2o molecules
  int numo2 = 0;                     // Used to store number of h2o molecules
  int numso2 = 0;                     // Used to store number of h2o molecules
  int i;                              // Loop index variable
  uint32 h_mem;                       // Used to hold handle to shared memory page
  sem_t s_procs_completed;            // Semaphores for completion of all processes
  chem_semaphore cs;                  // molecule/reaction semaphores
  char s_procs_completed_str[10];     // Used as command-line argument to pass mem_handle to new processes
  char h_mem_str[10];                 // Used as command-line argument to pass mem_handle to new processes 
  char h2o_str[10];                 // Used as command-line argument to pass mem_handle to new processes 
  char so4_str[10];                 // Used as command-line argument to pass mem_handle to new processes 
  char h2_str[10];                 // Used as command-line argument to pass mem_handle to new processes 
  char o2_str[10];                 // Used as command-line argument to pass mem_handle to new processes 
  char so2_str[10];                 // Used as command-line argument to pass mem_handle to new processes 

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  numh2o = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("Creating %d processes\n", numh2o);

  numso4 = dstrtol(argv[2], NULL, 10); // the "10" means base 10
  Printf("Creating %d processes\n", nums04);

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
  
  //Calculate number of each molecules
  numh2 = numh2o;
  numo2 = numh2o / 2 + numso4;
  numso2 = numso4;

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(h2o, h2o_str);
  ditoa(so4, so4_str);
  ditoa(h2, h2_str);
  ditoa(o2, o2_str);
  ditoa(so2, so2_str);

  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  process_create(PRODUCER_TO_RUN, s_procs_completed_str, h2o_str, NULL); // h2o
  process_create(PRODUCER_TO_RUN, s_procs_completed_str, so4_str, NULL); // so4
  process_create(CONSUMER_TO_RUN, s_procs_completed_str, h2o_str, h2_str, o2_str, NULL); // reaction 1
  process_create(CONSUMER_TO_RUN, s_procs_completed_str, so4_str, so2_str, o2_str, NULL); // reaction 2
  process_create(CONSUMER_TO_RUN, s_procs_completed_str, h2_str, o2_str, so2_str, NULL); // reaction 3
  Printf("Process %d created\n", i);

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad condition s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");

  return;
}
