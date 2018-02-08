#ifndef __USERPROG__
#define __USERPROG__

typedef struct missile_code {
    int numprocs;
    char really_important_char;
} missile_code;

typedef struct chem_semaphore {
  sem_t h2o;                          // Semaphores for h2o molecules
  sem_t so4;                          // Semaphores for so4 molecules
  sem_t h2;							  // Semaphores for h2 molecules
  sem_t o2;							  // Semaphores for o2 molecules
  sem_t so2;						  // Semaphores for so2 molecules
  sem_t h2so4;						  // Semaphores for h2so4 molecules
} chem_semaphore;

#define FILENAME_TO_RUN "spawn_me.dlx.obj"
#define PRODUCER_TO_RUN "spawn_producer.dlx.obj"
#define CONSUMER_TO_RUN "spawn_consumer.dlx.obj"
#define H2O_TO_RUN		"injectH2O.dlx.obj"
#define SO4_TO_RUN		"injectSO4.dlx.obj"
#define R1_TO_RUN		"reaction1.dlx.obj"
#define R2_TO_RUN		"reaction2.dlx.obj"
#define R3_TO_RUN		"reaction3.dlx.obj"
#endif