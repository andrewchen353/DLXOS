#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char* argv[])
{
  uint32 h_mem;             // Handle to the shared memory page
  sem_t s_procs_completed;  // Semaphore to signal the original process that we're done
  chem_semaphore *cs;       // Structure to hold semaphores
  int numh2o;               // Number of h2o molecules

  if (argc != 4) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[2], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  numh2o = dstrtol(argv[3], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((cs = (chem_semaphore *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
    Exit();
  } 

  // inject h2o molecules
  while (numh2o > 0) {
    if(sem_signal(cs->h2o) != SYNC_SUCCESS) {
      Printf("Bad semaphore h2o (%d) in ", cs->h2o); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }

    Printf("Process %d injected an H2O molecule\n", getpid());
    numh2o--;
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("Producer (H2O): PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
