#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char* argv[])
{
  uint32 h_mem;             // Handle to the shared memory page
  sem_t s_procs_completed;  // Semaphore to signal the original process that we're done
  chem_semaphore *cs;       // Structure to hold semaphores
  int numr1;               // Number of h2o molecules

  if (argc != 4) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[2], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  numr1 = dstrtol(argv[3], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((cs = (chem_semaphore *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
    Exit();
  } 

  // Reaction 1
  while (numr1 > 0) {
    // consume 2h2o
    if(sem_wait(cs->h2o) != SYNC_SUCCESS) {
      Printf("Bad semaphore H2O (%d) in ", cs->h2o); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }
    if(sem_wait(cs->h2o) != SYNC_SUCCESS) {
      Printf("Bad semaphore H2O (%d) in ", cs->h2o); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }

    // produce 2h2
    if(sem_signal(cs->h2) != SYNC_SUCCESS) {
      Printf("Bad semaphore H2 (%d) in ", cs->h2); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }
    if(sem_signal(cs->h2) != SYNC_SUCCESS) {
      Printf("Bad semaphore H2 (%d) in ", cs->h2); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }

    // produce o2
    if(sem_signal(cs->o2) != SYNC_SUCCESS) {
      Printf("Bad semaphore O2 (%d) in ", cs->o2); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }
    Printf("Process %d has created an O2 molecule from Reaction 1\n", getpid());

    numr1--;
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("Consumer (Reaction 1): PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
