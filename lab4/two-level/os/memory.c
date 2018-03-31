//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page = 2^21 / 4096 = 512
static uint32 freemap[MEM_MAX_PAGES / 32];

static uint32 pagestart;
static int nfreepages;
//static int freemapmax; 

// Declaring L2 page table
static l2_pagetable l2_num_tables[MEM_L2TABLE_SIZE]; // TODO this should be MEM_L1TABLE_SIZE (2) because l2 table size references to the array inside the structure

//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
/*static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}
*/
//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}


//----------------------------------------------------------------------
//
//	MemoryInitModule
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------
void MemoryModuleInit() {
  int i;
  int page, page_idx, page_bit;
  uint32 pagemask;
  uint32 onemask;
  uint32 l1_page;
  uint32 l2_page;

  dbprintf('m', "\nEnter MemoryModuleInit (%u)\n", GetCurrentPid());
  
  //**important
  // L1_pagenum * L2_pagenum / 32 = page index
  // do something to get the offset within the index
  pagestart = (lastosaddress >> MEM_L2FIELD_FIRST_BITNUM) + 1;
  l2_page = pagestart & 0xFF;
  l1_page = pagestart >> 8;
  page = (l1_page * 256) + l2_page;
  page_idx = page/32; //TODO look over this to make sure it is in freemap array
  page_bit = page % 32;

  dbprintf('m', "MemoryModuleInit:\nl astosaddress: %x\n", lastosaddress);
  dbprintf('m', "pagestart: %x\n", pagestart);
  dbprintf('m', "page_idx: %x\n", page_idx);
  dbprintf('m', "page_bit: %x\n", page_bit);
  dbprintf('m', "l1_page: %x\n", l1_page);
  dbprintf('m', "l2_page: %x\n", l2_page);
  
  pagemask = 0xFFFFFFFF << page_bit;
  onemask = 0xFFFFFFFF;
  dbprintf('m', "page_mask: %x\n", pagemask);
  
  for(i = 0; i < MEM_L2TABLE_SIZE; i++)
    l2_num_tables[i].inuse = 0;

  for(i = MEM_MAX_PAGES / 32 - 1; i >= 0; i--)
  {
    if (i > page_idx)
      freemap[i] = onemask;
    else
      freemap[i] = 0;
  }

  freemap[page_idx] = pagemask;

  nfreepages = MEM_MAX_PAGES - page; //FIXME
  dbprintf('m', "nfreepages: %x\n", nfreepages);
  dbprintf('m', "Leaving MemoryModuleInit (%d)\n", GetCurrentPid());
  return; //TODO check to make sure it works*/
}


//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
  uint32 offset;
  uint32 entry;
  uint32 phys_addr;
  uint32 l1_pagenum;
  uint32 l2_pagenum;

  dbprintf('m', "\nEntering MemoryTranslateUserToSystem (%d)\n", GetCurrentPid()); 
  if (addr > MEM_MAX_VIRTUAL_ADDRESS)
  {
    ProcessKill();
    return MEM_FAIL;
  }

  l1_pagenum = addr >> MEM_L1FIELD_FIRST_BITNUM; 
  l2_pagenum = (addr >> MEM_L2FIELD_FIRST_BITNUM) & 0xFF;
  offset = (addr & 0x00000FFF);
  //entry = ((uint32*)pcb->pagetable[l1_pagenum])[l2_pagenum];
  entry = ((l2_pagetable *)pcb->pagetable[l1_pagenum])->table[l2_pagenum]; //FIXME
  //entry = *(pcb->pagetable[l1_pagenum] + l2_pagenum);

  dbprintf('m', "addr: %x\n", addr);
  dbprintf('m', "l1_pagenum: %x\n", l1_pagenum);
  dbprintf('m', "l2_pagenum: %x\n", l2_pagenum);
  dbprintf('m', "offset: %x\n", offset);
  dbprintf('m', "entry: %x\n", entry);

  if (!(entry & MEM_PTE_VALID))
  {
    dbprintf('m', "MemoryTranslateUserToSystem calling page fault handler\n");
    pcb->currentSavedFrame[PROCESS_STACK_FAULT] = addr;
    return MemoryPageFaultHandler(pcb); //FIXME more may need to be done
  }
  phys_addr = (entry & 0xFFFFF000) | offset;
  dbprintf('m', "phys_addr: %x\n", phys_addr);
  dbprintf('m', "Leaving MemoryTranslateUserToSystem (%u)\n", GetCurrentPid());
  
  return phys_addr;
}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.igned char *)MemoryTranslateUserToSyste
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int		bytesCopied = 0;  // Running counter
  int		bytesToCopy;      // Used to compute number of bytes left in page to be copied

  dbprintf('m', "\nEntering MemoryMoveBetweenSpaces (%d)\n", GetCurrentPid());
  while (n > 0) {
    // Translate current user page to system address.e001  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);

    // If we could not translate address, exit now
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);
    
    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  dbprintf('m', "Leaving MemoryMoveBetweenSpaces (%d)\n", GetCurrentPid());
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  dbprintf('m', "\nEntering and leaving MemoryCopySystemToUser (%d)\n", GetCurrentPid());
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  dbprintf('m', "\nEntering MemoryCopyUserToSystem (%d)\n", GetCurrentPid());
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference. 
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) {

  uint32 addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
  uint32 userstackaddr = pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER];
  uint32 vl1_pagenum = addr >> MEM_L1FIELD_FIRST_BITNUM;
  uint32 vl2_pagenum = (addr >> MEM_L2FIELD_FIRST_BITNUM) & 0xFF;
  uint32 vpagenum = vl1_pagenum * 256 + vl2_pagenum;
  uint32 sl1_pagenum = userstackaddr >> MEM_L1FIELD_FIRST_BITNUM;
  uint32 sl2_pagenum = (userstackaddr >> MEM_L2FIELD_FIRST_BITNUM) & 0xFF;
  uint32 stackpagenum = sl1_pagenum * 256 + sl2_pagenum;
  uint32 ppagenum;
  uint32* l2entry;

  dbprintf('m', "\nEntering MemoryPageFaultHandler (%d)\n", GetCurrentPid());
  dbprintf('m', "Number of free pages = %u\n", nfreepages);

  /* // segfault if the faulting address is not part of the stack */
  if (vpagenum < stackpagenum) {
    dbprintf('m', "addr = %x\nsp = %x\n", addr, pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]);
    printf("vpagenum = %x, stackpagenum = %x \n", vpagenum, stackpagenum);
    printf("FATAL ERROR (%d): segmentation fault at page address %x\n", GetPidFromAddress(pcb), addr);
    ProcessKill();
    return MEM_FAIL;
  }

  ppagenum = MemoryAllocPage();
  //l2entry = pcb->pagetable[vl1_pagenum] + vl2_pagenum;
  //*l2entry = MemorySetupPte(ppagenum);
  ((l2_pagetable *)pcb->pagetable[vl1_pagenum])->table[vl2_pagenum] = MemorySetupPte(ppagenum);  
  l2_num_tables[vl2_pagenum].inuse = 1;
  dbprintf('m', "Returning from page fault handler\n");
  dbprintf('m', "Leaving MemoryPageFaultHandler (%d)\n", GetCurrentPid());
  return MEM_SUCCESS;
}


//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------
int MemoryAllocPage(void) {
  int i, j;
  uint32 page_idx;
  uint32 page_bit;
  uint32 page_bunch;  
  int virtual_page;

  dbprintf('m', "\nEntering MemoryAlloc (%d)\n", GetCurrentPid());

  if(!nfreepages) return 0;
  
  for (i = 0; i < MEM_MAX_PAGES / 32; i++) {
    page_bunch = freemap[i];
    if (page_bunch) {
      page_idx = i;
      break;
    }
  }
  page_bit = 0;
  for (j = 0; j < 32; j++) {
    if (page_bunch & 0x1) {
      page_bit = j;
      break;
    }
    page_bunch = page_bunch >> 0x1;
  }
  dbprintf('m', "page_bit: %d\n", page_bit);
  dbprintf('m', "freemap[%d] before: %x\n", page_idx, freemap[page_idx]);
  // set allocated page bit to 0
  if (page_bit == 31)
    freemap[page_idx] = 0x0;
  else
    freemap[page_idx] = freemap[page_idx] & (negativeone << (page_bit + 1) | ((1 << page_bit) - 1));
 
  nfreepages--;
 
  dbprintf('m', "freemap[%d] after: %x\n", page_idx, freemap[page_idx]);

  virtual_page = page_idx * 32 + page_bit;
  l2_num_tables[virtual_page].inuse = 1;  // TODO virtual page could be a number between 0 and 512? This doesnt make sense if we have only two l2_page tables.
  
  dbprintf('m', "virtual_page: %d\n", virtual_page);
  dbprintf('m', "Leaving MemoryAlloc (%d)", GetCurrentPid());
  
  if (virtual_page > MEM_L2TABLE_SIZE * 2) // using only 2 L2 pagetables
    return MEM_FAIL;
  else
    return virtual_page;
}


uint32 MemorySetupPte (uint32 page) {
  dbprintf('m', "\nEnter and leave MemorySetupPte (%d)\n", GetCurrentPid());
  return page * MEM_PAGESIZE + MEM_PTE_VALID;
}


void MemoryFreePage(uint32 page) {
  uint32 page_idx = page / 32;
  uint32 page_bit = page - page_idx * 32;
  uint32 page_mask = 1 << page_bit;
  
  dbprintf('m', "\nEnter MemoryFreePage (%d)\n", GetCurrentPid());
  freemap[page_idx] |= page_mask;
  dbprintf('m', "Leave MemoryFreePage (%d)\n", GetCurrentPid());
  nfreepages++;

  l2_num_tables[page].inuse = 0; // TODO doesn't make sense for the same reason as above
}

int malloc() {
  return 0;
//void* malloc(int memsize) {
  //return NULL;
}

int mfree() {
//int mfree(void* ptr) {
  return 0;
}
