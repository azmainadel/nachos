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
#include "thread.h"
#include "table.h"
#include "console.h"
#include "synch.h"


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

void PCInc(){
	int pc = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, pc);
	pc = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, pc);
	pc += 4;
	machine->WriteRegister(NextPCReg, pc);
}

void Fork(void* arg){
	(currentThread->space)->InitRegisters();
	(currentThread->space)->RestoreState();
	machine->Run();

	return;
}

extern Console* console;
extern Semaphore* readAvail;
extern Semaphore* writeDone;
Lock RWLock("readWriteLock");


void ExceptionHandler(ExceptionType which){
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
    	DEBUG('a', "Shutdown, initiated by user program.\n");
       	interrupt->Halt();
    }

//-------------------------------EXEC--------------------------------------
    else if(((which == SyscallException) && (type == SC_Exec))){
    	int param = machine->ReadRegister(4);

        char fileName[100];
        int i = 0;

        while(fileName[i] != '\0'){
            machine->ReadMem(param + i, 1, (int*)&fileName[i]);
            i++;
        }
        // printf("%s\n", fileName);
    	OpenFile* executable = fileSystem->Open(fileName);

        if (executable == NULL){
            printf("Could not open file: %s\n", fileName);
    		machine->WriteRegister(2, 0);
    		PCInc();
            return;
        }

    	Thread* thread = new Thread(fileName);
    	AddrSpace* space = new AddrSpace(executable);
    	thread->space = space;
    	int ret = pTable->Alloc((void*)thread);

    	if(ret == -1){
    		printf("Allocation failed\n" );
    		PCInc();
    		return;
    	}

        delete executable;

    	thread->setID(ret);
    	machine->WriteRegister(2, ret);

        thread->Fork(Fork, NULL);
    	PCInc();
        // printf("EXEC Test\n");
    	return;
    }

//-------------------------------EXIT--------------------------------------
    else if((which == SyscallException) && (type == SC_Exit)){
        printf("\nEXIT test\n" );
        fflush(stdout);

        int param = machine->ReadRegister(4);
        int index = currentThread->getID();

        pTable->Release(index);
        currentThread->space->ReleaseMemory();
        currentThread->Finish();
        PCInc();

        return (void)param;
    }

//-------------------------------READ--------------------------------------
    else if((which == SyscallException) && (type == SC_Read)){

        unsigned int address = machine->ReadRegister(4);
        unsigned int size = machine->ReadRegister(5);
        unsigned int id = machine->ReadRegister(6);

		char* buffer = new char[size];

        for(int i = 0; i < size; i++){
            readAvail->P();
            buffer[i] = console->GetChar();
        }
        buffer[size] = '\0';

        for(int i = 0; i < size; i++){
            machine->WriteMem(address,1, (int)buffer[i]);
            address++;
        }

        machine->WriteRegister(2, size);
        bzero(buffer, sizeof(char) * size);

        PCInc();

        return;
    }

//-------------------------------WRITE--------------------------------------
    else if((which == SyscallException) && (type == SC_Write)){

        unsigned int address = machine->ReadRegister(4);
        unsigned int size = machine->ReadRegister(5);

		char* buffer = new char[size];

        for(int i = 0; i < size; i++){
            int c;
            machine->ReadMem(address, 1, &c);
            buffer[i] = (char)c;
            address++;
        }

        for(int i = 0; i < size; i++){
            console->PutChar(buffer[i]);
            writeDone->P();
        }

        PCInc();
        return;
    }

    else {
    	printf("Unexpected user mode exception %d %d\n", which, type);
    	ASSERT(false);
    }
}
