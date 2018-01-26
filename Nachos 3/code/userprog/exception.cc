// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------
#include "processtable.h"
#include "memorymanager.h"
#include "synchconsole.h"
#include "machine.h"

extern ProcessTable *processTable;
extern Lock *syscallLock;
extern MemoryManager *memoryManager;
extern MemoryManager *swapMemoryManager;
extern SynchConsole * synchConsole;

void processCreator(void *arg)
{
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
	//printf("fork successfully executed\n");
	// load page table register
	machine->Run(); // jump to the user progam
	ASSERT(false); // machine->Run never returns;
}

void ExitProcess()
{
	syscallLock->Acquire();
	int size = machine->pageTableSize;

	for(int i = 0; i < size; ++i)
		if(machine->pageTable[i].valid) memoryManager->FreePage(machine->pageTable[i].physicalPage);
	processTable->Release(currentThread->id);

	printf("exited with exit code %d\n", machine->ReadRegister(4));
	syscallLock->Release();

	if(processTable->nowSize == 0)
		interrupt->Halt();
	else
		currentThread->Finish();
}

void updateAllPCReg()
{
	/* routine task – do at last -- generally manipulate PCReg,
	PrevPCReg, NextPCReg so that they point to proper place*/
	int pc;
	pc=machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg,pc);
	pc=machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg,pc);
	pc += 4;
	machine->WriteRegister(NextPCReg,pc);
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if (which == SyscallException) {
    	if(type == SC_Halt){
			DEBUG('a', "Shutdown, initiated by user program.\n");
		   	interrupt->Halt();
	    }
	    else if(type == SC_Exec){
	   	    //IntStatus oldLevel = interrupt->SetLevel(IntOff);   //disable interrupt

	    	syscallLock->Acquire();
	    	int bufadd = machine->ReadRegister(4);

	    	char *filename = new char[100];
			//find a proper place to free this allocation
			int ch;
			if(!machine->ReadMem(bufadd,1,&ch))
				return;
			unsigned int i=0;
			while( ch != 0 )
			{
				filename[i] = (char)ch;
				bufadd += 1;
				i++;
				if(!machine->ReadMem(bufadd,1,&ch))
					return;
			}
			filename[i]=(char)0;
			/* now filename contains the file */

			OpenFile *executable = fileSystem->Open(filename);
			AddrSpace *space = new AddrSpace(executable);
			Thread * t = new Thread("tname");
			t->space = space;

			delete executable;

			t->id = processTable->Alloc( (void *) t );
			unsigned int processId = t->id;

			syscallLock->Release();

			t->Fork(processCreator, (void *) &processId);

			/* return the process id for the newly created process, return value
			is to write at R2 */
			machine->WriteRegister(2, processId);
			updateAllPCReg();
			//printf("thread with id %d created\n", processId);
			//(void) interrupt->SetLevel(oldLevel);               //re enable interrupt
	    }
	    else if(type == SC_Exit)
	    {
	    	ExitProcess();
	    }
	    else if(type == SC_Read)
	    {
	    	syscallLock->Acquire();
	    	printf("in reading\n");
	    	int addr = machine->ReadRegister(4);
	    	int size = machine->ReadRegister(5);
	    	synchConsole->Read(addr, size, currentThread->id);
	    	syscallLock->Release();
	    	updateAllPCReg();
	    }
	    else if(type == SC_Write)
	    {
	    	syscallLock->Acquire();
	    	printf("in writing\n");
	    	int addr = machine->ReadRegister(4);
	    	int size = machine->ReadRegister(5);
	    	synchConsole->Write(addr, size, currentThread->id);
	    	syscallLock->Release();
	    	updateAllPCReg();
	    }
	    else {
	    	printf("Unexpected user mode exception %d %d\n", which, type);
	    	ExitProcess();
	    }
    }
    else if(which == PageFaultException)
    {
    	// printf("PageFaultException\n");

		int address = machine->ReadRegister(39);
		int virtualPageNumber = address / PageSize;
		int physicalPageNumber;

		stats->numPageFaults++;

        if(memoryManager->IsAnyPageFree() == true){
			// physicalPageNumber = memoryManager->AllocPage(); //TASK: DEMAND PAGING
			physicalPageNumber = memoryManager->AllocPage(currentThread->id, &(machine->pageTable[virtualPageNumber])); //TASK: RANDOM REPLACEMENT
		}
		else{
            // for(j = 0; j < i; ++j) memoryManager->FreePage(pageTable[j].physicalPage);
            // ASSERT(false);

			// physicalPageNumber = memoryManager->AllocByForce(); //TASK: RANDOM REPLACEMENT
			physicalPageNumber = memoryManager->AllocByForceLRU(); //TASK: LRU REPLACEMENT

			currentThread->space->saveIntoSwapSpace(memoryManager->entries[physicalPageNumber]->virtualPage);
			memoryManager->entries[physicalPageNumber] = &machine->pageTable[virtualPageNumber];
			memoryManager->processMap[physicalPageNumber] = currentThread->id;
        }

		currentThread->space->loadIntoFreePage(address, physicalPageNumber);

    	// ExitProcess();
    }
    else if(which == ReadOnlyException)
    {
    	printf("ReadOnlyException\n");
    	ExitProcess();
    }
    else if(which == BusErrorException)
    {
    	printf("BusErrorException\n");
    	ExitProcess();
    }
    else if(which == AddressErrorException)
    {
    	printf("AddressErrorException\n");
    	ExitProcess();
    }
    else if(which == OverflowException)
    {
    	printf("OverflowException\n");
    	ExitProcess();
    }
    else if(which == NumExceptionTypes)
    {
    	printf("NumExceptionTypes\n");
    	ExitProcess();
    }
    else if(which == IllegalInstrException)
    {
    	// printf("IllegalInstrException\n");
    	ExitProcess();
    }
    else {
		printf("Unexpected user mode exception %d %d\n", which, type);
		ExitProcess();
    }
}
