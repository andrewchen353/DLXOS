#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"
#define PART2 "part2.dlx.obj"
#define PART3 "part3.dlx.obj"
#define PART5 "part5.dlx.obj"
#define PART6 "part6.dlx.obj"

void main (int argc, char *argv[])
{
  int num_hello_world = 0;             // Used to store number of processes to create
  int i;                               // Loop index variable
  sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes

  if (argc != 2) {
    Printf("Usage: %s <number of hello world processes to create>\n", argv[0]);
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  num_hello_world = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("makeprocs (%d): Creating %d hello_world processes\n", getpid(), num_hello_world);

  // Create semaphore to not exit this process until all other processes
  // have signalled that they are complete
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes. We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);

  Printf("\n-------------------------------------Part1-------------------------------------------\n");
  // Create Hello World processes
  Printf("makeprocs (%d): Creating 1 hello world\n", getpid());
  process_create(HELLO_WORLD, s_procs_completed_str, NULL);
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }
  Printf("-------------------------------------------------------------------------------------\n");

  /*
  // Uncomment this section to run part 2
  Printf("\n-------------------------------------Part2-------------------------------------------\n");
  process_create(PART2, s_procs_completed_str, NULL);
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }
  Printf("-------------------------------------------------------------------------------------\n");
  */

  Printf("\n-------------------------------------Part3-------------------------------------------\n");
  process_create(PART3, s_procs_completed_str, NULL);
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }
  Printf("-------------------------------------------------------------------------------------\n");

  Printf("\n-------------------------------------Part4-------------------------------------------\n");
  // Create Hello World processes
  Printf("makeprocs (%d): Creating 100 hello world's in a row\n", getpid());
  for(i=0; i<100; i++) {
    Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
    process_create(HELLO_WORLD, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
  }
  Printf("-------------------------------------------------------------------------------------\n");
  
  Printf("\n-------------------------------------Part5-------------------------------------------\n");
  // Create semaphore to not exit this process until all other processes
  // have signalled that they are complete
  if ((s_procs_completed = sem_create(-30+1)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes. We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);

  for(i=0; i<30; i++) {
    Printf("makeprocs (%d), creating process num: %d\n", getpid(), i);
    process_create(PART5, s_procs_completed_str, NULL);
  }

  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }
  Printf("-------------------------------------------------------------------------------------\n");

/*  
  // Uncomment this section to run part 6
  Printf("\n-------------------------------------Part6-------------------------------------------\n");
  // Create semaphore to not exit this process until all other processes
  // have signalled that they are complete
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes. We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);
  process_create(PART6, s_procs_completed_str, NULL);
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }
  Printf("-------------------------------------------------------------------------------------\n");
*/

  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
