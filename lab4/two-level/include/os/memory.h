#ifndef	_memory_h_
#define	_memory_h_

// Put all your #define's in memory_constants.h
#include "memory_constants.h"

extern int lastosaddress; // Defined in an assembly file

// l2 pagetable
typedef struct l2_pagetable {
  int inuse;
  uint32 table[MEM_L2TABLE_SIZE];
} l2_pagetable;

//--------------------------------------------------------
// Existing function prototypes:
//--------------------------------------------------------

int MemoryGetSize();
void MemoryModuleInit();
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr);
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir);
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryPageFaultHandler(PCB *pcb);

//---------------------------------------------------------
// Put your function prototypes here
//---------------------------------------------------------
// All function prototypes including the malloc and mfree functions go here
int MemoryAllocPage(void);
uint32 MemorySetupPte(uint32 page);
void MemoryFreePage(uint32 page);
uint32* GetAddressL2();
//void checkAllocatel2pt(PCB* pcb, uint32 l1_page);
void ResetPTInUse(PCB* pcb);
int malloc();
int mfree();
//void* malloc(int memsize);
//int mfree(void* ptr);

#endif	// _memory_h_
