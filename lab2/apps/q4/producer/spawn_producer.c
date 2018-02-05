#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char* argv[])
{
  circular_buff *cb;        // Used to access circular buffer in shared memory page
  uint32 h_mem;             // Handle to the shared memory page
  cond_t c_procs_completed;  // Condition to signal the original process that we're done
  lock_t lock;              // Lock
  char* c = "Hello world";  // Character to be inserted into buffer
  int i = 0;

  if (argc != 4) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  c_procs_completed = dstrtol(argv[2], NULL, 10);
  lock = dstrtol(argv[3], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((cb = (circular_buff *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
 
  // Now print a message to show that everything worked
  while (i < 11)
  {
    lock_acquire(lock);
    if ((cb->head + 1) % cb->size != cb->tail)
    {
      cb->buff[cb->head] = c[i];
      Printf("Producer %d inserted: %c\n", getpid(), cb->buff[cb->head]);
      cb->head = (++(cb->head))%cb->size;
      i++;
    }
    lock_release(lock);
  }

  // Signal the conditon to tell the original process that we're done
  Printf("producer: PID %d is complete.\n", getpid());
  if(cond_signal(c_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad condition c_procs_completed (%d) in ", c_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
