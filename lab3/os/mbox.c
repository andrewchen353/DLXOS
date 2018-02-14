#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

static mbox_message mm[MBOX_NUM_BUFFERS];
static mbox mailbox[MBOX_NUM_MBOXES];

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
  
  for (m = 0; m < MBOX_NUM_MBOXES; m++) 
    mailbox[m].inuse = 0;
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

      mailbox[m].lock = lock_create();
      if (mailbox[m].lock == SYNC_FAIL)
      {
        printf("FATAL ERROR: could not create lock\n");
        exitsim();
      }

      mailbox[m].notFull = cond_create(mailbox[m].lock);
      if (mailbox[m].notFull == INVALID_COND)
      {
        printf("FATAL ERROR: could not create condition notFull\n");
        exitsim();
      }

      mailbox[m].notEmpty = cond_create(mailbox[m].lock);
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

  if (m == MBOX_NUM_MBOXES) return MBOX_FAIL;

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
  lock_t l;
  if ((l = lock_acquire(mailbox[handle].lock)) != SYNC_SUCCESS)
  {
    printf("FATAL ERROR: could not open mailbox\n");
    //exitsim();
    return MBOX_FAIL;
  }
  
  mailbox[handle].procs[getpid()] = true;
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
  lock_t l;
  int i;

  mailbox[handle].procs[getpid()] = false;

  for (i = 0; i < PROCESS_MAX_PROCS; i++)
    if (mailbox[handle].procs[i] == true)
      break
  if (i == PROCESS_MAX_PROCS)
    mailbox[handle].inuse = 0;

  if ((l = lock_release(mailbox[handle].lock)) != SYNC_SUCCESS)
  {
    printf("FATAL ERROR: could not close mailbox\n");
    return MBOX_FAIL;
  }
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
  return MBOX_FAIL;
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
  return MBOX_FAIL;
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
  return MBOX_FAIL;
}
