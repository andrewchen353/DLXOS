#ifndef __USERPROG__
#define __USERPROG__

typedef struct missile_code {
    int numprocs;
    char really_important_char;
} missile_code;

typedef struct chem_semaphore {
  sem_t h2o;                          // Semaphores for h2o molecules
  sem_t so4;                          // Semaphores for so4 molecules
  sem_t r1;                           // Semaphores for reaction 1 2H2O->2H2+O2
  sem_t r2;                           // Semaphores for reaction 2 SO4->SO2+O2
  sem_t r3;                           // Semaphores for reaction 3 H2+O2+SO2->H2SO4
 
} chem_semaphore;

#define FILENAME_TO_RUN "spawn_me.dlx.obj"
#define PRODUCER_TO_RUN "spawn_producer.dlx.obj"
#define CONSUMER_TO_RUN "spawn_consumer.dlx.obj"

#endif
