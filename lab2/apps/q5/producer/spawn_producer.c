#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char* argv[])
{
  circular_buff *cb;        // Used to access circular buffer in shared memory page
  uint32 h_mem;             // Handle to the shared memory page
  cond_t c_full;            // Condition that the buffer is full
  cond_t c_empty;           // Condition that the buffer is empty
  sem_t s_procs_completed;  // Semaphore to signal the original process that we're done
  lock_t lock;              // Lock
  char* c = "Hello world";  // Character to be inserted into buffer
  int i = 0;

  if (argc != 6) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  lock = dstrtol(argv[3], NULL, 10);
  c_full = dstrtol(argv[4], NULL, 10);
  c_empty = dstrtol(argv[5], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((cb = (circular_buff *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
 
  // Now print a message to show that everything worked
  while (i < 11)
  {
    //cond_wait(c_empty);
    lock_acquire(lock);
    if ((cb->head + 1) % cb->size != cb->tail)
    {
      cb->buff[cb->head] = c[i];
      Printf("Producer %d inserted: %c\n", getpid(), cb->buff[cb->head]);
      cb->head = (++(cb->head))%cb->size;
      i++;
      cond_signal(c_empty);
    }
    else
      cond_wait(c_full);
    lock_release(lock);
    //cond_signal(c_full);
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("producer: PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
