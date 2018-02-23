#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

static mbox_message mm[MBOX_NUM_BUFFERS];
static mbox mailbox[MBOX_NUM_MBOXES];

//////LOCK ACQUIRE AND RELEASE EVERY TIME ACCESS MAILBOX!!!!!!!!!!!!!!!

//-------------------------------------------------------
//
// void MboxModuleInit();
//
// Initialize all mailboxes.  This process does not need
// to worry about synchronization as it is called at boot
// time.  Only initialize necessary items here: you can
// initialize others in MboxCreate.  In other words, 
// don't waste system resources like locks and semaphores
// on unused mailboxes.
//
//-------------------------------------------------------

void MboxModuleInit() {
  mbox_t m;
  uint32 intrval;
  
  intrval = DisableIntrs();
  for (m = 0; m < MBOX_NUM_MBOXES; m++) 
    mailbox[m].inuse = 0;
  for (m = 0; m < MBOX_NUM_BUFFERS; m++)
    mm[m].inuse = 0;

  RestoreIntrs(intrval);
}

//-------------------------------------------------------
//
// mbox_t MboxCreate();
//
// Allocate an available mailbox structure for use. 
//
// Returns the mailbox handle on success
// Returns MBOX_FAIL on error.
//
//-------------------------------------------------------
mbox_t MboxCreate() {
  mbox_t m;
  int i;
  uint32 intrval;

  intrval = DisableIntrs();
  for(m = 0; m < MBOX_NUM_MBOXES; m++)
  {
    if (!mailbox[m].inuse)
    {
      mailbox[m].inuse = 1;

      for (i = 0; i < PROCESS_MAX_PROCS; i++)
        mailbox[m].procs[i] = 0;

      mailbox[m].lock = LockCreate();
      if (mailbox[m].lock == SYNC_FAIL)
      {
        printf("FATAL ERROR: could not create lock\n");
        exitsim();
      }

      mailbox[m].notFull = CondCreate(mailbox[m].lock);
      if (mailbox[m].notFull == INVALID_COND)
      {
        printf("FATAL ERROR: could not create condition notFull\n");
        exitsim();
      }

      mailbox[m].notEmpty = CondCreate(mailbox[m].lock);
      if (mailbox[m].notEmpty == INVALID_COND)
      {
        printf("FATAL ERROR: could not create condition notEmpty\n");
        exitsim();
      }

      if (AQueueInit(&mailbox[m].messageQ) != QUEUE_SUCCESS)
      {
        printf("FATAL ERROR: could not initialize queue\n");
        exitsim();
      }
      break;
    }
  }
  RestoreIntrs(intrval);

  if (m == MBOX_NUM_MBOXES) 
  {
    printf("ERROR: No more mailboxes available\n");
    return MBOX_FAIL;
  }

  return m;
}

//-------------------------------------------------------
// 
// int MboxOpen(mbox_t);
//
// Open the mailbox for use by the current process.  Note
// that it is assumed that the internal lock/mutex handle 
// of the mailbox and the inuse flag will not be changed 
// during execution.  This allows us to get the a valid 
// lock handle without a need for synchronization.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxOpen(mbox_t handle) {
  int intrval;
  intrval = DisableIntrs();
  if (LockHandleAcquire(mailbox[handle].lock) != SYNC_SUCCESS)
  {
    printf("FATAL ERROR: could not open mailbox\n");
    return MBOX_FAIL;
  }

  // empty queue if unused
  if (!mailbox[handle].inuse)
    if (AQueueInit(&mailbox[handle].messageQ) != QUEUE_SUCCESS)
    {
      printf("ERROR: could initialize mailbox queue\n");
      return MBOX_FAIL;
    }

  mailbox[handle].procs[GetCurrentPid()] = 1;

  if (LockHandleRelease(mailbox[handle].lock) != SYNC_SUCCESS)
  {
    printf("FATAL ERROR: could not close mailbox\n");
    return MBOX_FAIL;
  }

  RestoreIntrs(intrval); 

  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxClose(mbox_t);
//
// Close the mailbox for use to the current process.
// If the number of processes using the given mailbox
// is zero, then disable the mailbox structure and
// return it to the set of available mboxes.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxClose(mbox_t handle) {
  int i;
  int intrval;

  intrval = DisableIntrs();

  if (LockHandleAcquire(mailbox[handle].lock) != SYNC_SUCCESS)
  {
    printf("FATAL ERROR: could not open mailbox\n");
    return MBOX_FAIL;
  }

  if (!mailbox[handle].inuse)
  {
    printf("ERROR: mailbox given is not inuse\n");
    return MBOX_FAIL;
  }

  mailbox[handle].procs[GetCurrentPid()] = 0;

  for (i = 0; i < PROCESS_MAX_PROCS; i++)
    if (mailbox[handle].procs[i] == 1)
      break;
  if (i == PROCESS_MAX_PROCS)
    mailbox[handle].inuse = 0;

  if (LockHandleRelease(mailbox[handle].lock) != SYNC_SUCCESS)
  {
    printf("FATAL ERROR: could not close mailbox\n");
    return MBOX_FAIL;
  }
  RestoreIntrs(intrval); 
  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxSend(mbox_t handle,int length, void* message);
//
// Send a message (pointed to by "message") of length
// "length" bytes to the specified mailbox.  Messages of
// length 0 are allowed.  The call 
// blocks when there is not enough space in the mailbox.
// Messages cannot be longer than MBOX_MAX_MESSAGE_LENGTH.
// Note that the calling process must have opened the 
// mailbox via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxSend(mbox_t handle, int length, void* message) {
  int i;
  Link* l;
  int intrval;
  char* c;

  if (length > MBOX_MAX_MESSAGE_LENGTH) 
  {
    // message too long
    printf("ERROR: SEND Message length greather than max message length\n");
    return MBOX_FAIL;
  }

  intrval = DisableIntrs();
  // acquire lock
  if (LockHandleAcquire(mailbox[handle].lock) != SYNC_SUCCESS)
  {
    printf("FATAL ERROR: could not get lock for mailbox in send\n");
    return MBOX_FAIL;
  }

  if (!mailbox[handle].inuse)
  {
    printf("ERROR: SEND Mailbox is not inuse\n");
    return MBOX_FAIL;
  }

  // check queue not full
  if (mailbox[handle].messageQ.nitems >= MBOX_MAX_BUFFERS_PER_MBOX)
    if (CondHandleWait(mailbox[handle].notFull) != SYNC_SUCCESS)
    {
      printf("ERROR: SEND Mailbox is full?\n");
      return MBOX_FAIL;
    }

  for (i = 0; i < MBOX_NUM_BUFFERS; i++)
    if (!mm[i].inuse)
      break;
  if (i == MBOX_NUM_BUFFERS)
  {
    printf("ERROR: SEND No more mailboxes are available\n");
    return MBOX_FAIL;
  }

  c = mm[i].buffer;
  c = (char*) message;
  //*(mm[i].buffer) = (char *)message; //??????
  //mm[i].buffer = (char *)&message;
  /*for (j = 0; j < length; j++)
    mm[i].buffer[j] = (char)message[j];*/

  mm[i].length = length;
  mm[i].inuse = 1;
  if ((l = AQueueAllocLink((void*)&mm[i])) == NULL)
  {
    printf("FATAL ERROR: SEND could not allocate link for queue in MboxSend\n");
    exitsim();
  }
  if (AQueueInsertLast(&mailbox[i].messageQ, l) != QUEUE_SUCCESS)
  {
    printf("FATAL ERROR: SEND could not insert new link into queue in MBoxSend\n");
    exitsim();
  }

  if (CondHandleSignal(mailbox[handle].notEmpty) != SYNC_SUCCESS)
  {
    printf("ERROR: SEND Mailbox is empty?\n");
    return MBOX_FAIL;
  }

  // release lock
  if (LockHandleRelease(mailbox[handle].lock) != SYNC_SUCCESS)
  {
    printf("FATAL ERROR: could not release lock for mailbox in send\n");
    return MBOX_FAIL;
  }

  RestoreIntrs(intrval); 
 
  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxRecv(mbox_t handle, int maxlength, void* message);
//
// Receive a message from the specified mailbox.  The call 
// blocks when there is no message in the buffer.  Maxlength
// should indicate the maximum number of bytes that can be
// copied from the buffer into the address of "message".  
// An error occurs if the message is larger than maxlength.
// Note that the calling process must have opened the mailbox 
// via MboxOpen.
//   
// Returns MBOX_FAIL on failure.
// Returns number of bytes written into message on success.
//
//-------------------------------------------------------
int MboxRecv(mbox_t handle, int maxlength, void* message) {
  int intrval;
  mbox_message* m;
  Link* l;

  intrval = DisableIntrs();
  // acquire lock
  if (LockHandleAcquire(mailbox[handle].lock) != SYNC_SUCCESS)
  {
    printf("FATAL ERROR: could not get lock for mailbox in send\n");
    return MBOX_FAIL;
  }

  if (!mailbox[handle].inuse)
  {
    printf("ERROR: RECV Mailbox not in use\n");
    return MBOX_FAIL;  
  }

  if (maxlength < mm[handle].length)
  {
    printf("ERROR: RECV maxlength is too large %d %d\n", maxlength, mm[handle].length);
    return MBOX_FAIL;
  }

  //if queue->nitems == 0
  if (!mailbox[handle].messageQ.nitems)   
    if (CondHandleWait(mailbox[handle].notEmpty) != SYNC_SUCCESS)
    {
      printf("ERROR: RECV mailbox is empty?\n");
      return MBOX_FAIL;
    }

  // pop from queue
  if (!AQueueEmpty(&mailbox[handle].messageQ))
  {
    l = AQueueFirst(&mailbox[handle].messageQ);
    m = (mbox_message *)AQueueObject(l);
    if (AQueueRemove(&l) != QUEUE_SUCCESS)
    {
      printf("FATAL ERROR: RECV could not remove link from mailbox queue\n");
      exitsim();
    }
  }
  message = &(m->buffer);
  m->inuse = 0;
/*  for (i = 0; i < mm[handle].length; i++)
  {
    *message = mm[handle].buffer[i];
    message += 8;
  }*/

  if (CondHandleSignal(mailbox[handle].notFull) != SYNC_SUCCESS)
  {
    printf("ERROR: RECV mailbox is full?\n");
    return MBOX_FAIL;
  }

  // release lock
  if (LockHandleRelease(mailbox[handle].lock) != SYNC_SUCCESS)
  {
    printf("FATAL ERROR: could not release lock for mailbox in send\n");
    return MBOX_FAIL;
  }

  RestoreIntrs(intrval); 

  return MBOX_SUCCESS;
}

//--------------------------------------------------------------------------------
// 
// int MboxCloseAllByPid(int pid);
//
// Scans through all mailboxes and removes this pid from their "open procs" list.
// If this was the only open process, then it makes the mailbox available.  Call
// this function in ProcessFreeResources in process.c.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//--------------------------------------------------------------------------------
int MboxCloseAllByPid(int pid) {
  int i, j;
  int intrval;

  intrval = DisableIntrs();
  for (i = 0; i < MBOX_NUM_MBOXES; i++)
  {
    // acquire lock
    if (LockHandleAcquire(mailbox[i].lock) != SYNC_SUCCESS)
    {
      printf("FATAL ERROR: could not get lock for mailbox in send\n");
      return MBOX_FAIL;
    }

    mailbox[i].inuse = 0;
    mailbox[i].procs[pid] = 0;

    for (j = 0; j < PROCESS_MAX_PROCS; j++)
      if (mailbox[i].procs[j])
      {
        mailbox[i].inuse = 1;
        break;
      }
    
    // release lock
    if (LockHandleRelease(mailbox[i].lock) != SYNC_SUCCESS)
    {
      printf("FATAL ERROR: could not release lock for mailbox in send\n");
      return MBOX_FAIL;
    }
  }

  RestoreIntrs(intrval); 
  return MBOX_SUCCESS;
}
