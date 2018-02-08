#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char* argv[])
{
  uint32 h_mem;             // Handle to the shared memory page
  sem_t s_procs_completed;  // Semaphore to signal the original process that we're done
  chem_semaphore *cs;       // Structure to hold semaphores
  int numso4;               // Number of h2o molecules

  if (argc != 4) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[2], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  numso4 = dstrtol(argv[3], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((cs = (chem_semaphore *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
    Exit();
  } 

  // inject h2o molecules
  while (numso4 > 0) {
    if(sem_signal(cs->so4) != SYNC_SUCCESS) {
      Printf("Bad semaphore SO4 (%d) in ", cs->so4); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }

    Printf("Process %d injected an SO4 molecule\n", getpid());
    numso4--;
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("Producer (SO4): PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
