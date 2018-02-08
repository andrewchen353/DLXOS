#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char* argv[])
{
  uint32 h_mem;             // Handle to the shared memory page
  sem_t s_procs_completed;  // Semaphore to signal the original process that we're done
  chem_semaphore *cs;       // Structure to hold semaphores
  int numr2;               // Number of h2o molecules

  if (argc != 4) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[2], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  numr2 = dstrtol(argv[3], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((cs = (chem_semaphore *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
    Exit();
  } 

  // Reaction 2
  while (numr2 > 0) {
    // consume so4
    if(sem_wait(cs->so4) != SYNC_SUCCESS) {
      Printf("Bad semaphore SO4 (%d) in ", cs->so4); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }

    // produce so2
    if(sem_signal(cs->so2) != SYNC_SUCCESS) {
      Printf("Bad semaphore SO2 (%d) in ", cs->so2); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }

    // produce o2
    if(sem_signal(cs->o2) != SYNC_SUCCESS) {
      Printf("Bad semaphore O2 (%d) in ", cs->o2); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }
    Printf("Process %d has created an O2 molecule from Reaction 2\n", getpid());

    numr2--;
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("Consumer (Reaction 2): PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
