#ifndef __USERPROG__
#define __USERPROG__

typedef struct missile_code {
    int numprocs;
    char really_important_char;
} missile_code;

typedef struct circular_buff {
    int size;   // size of buffer
    //char* buff;
    char buff[11];
    //char* head;
    //char* tail;
    int head;
    int tail;
} circular_buff;

#define FILENAME_TO_RUN "spawn_me.dlx.obj"
#define PRODUCER_TO_RUN "spawn_producer.dlx.obj"
#define CONSUMER_TO_RUN "spawn_consumer.dlx.obj"

#endif
