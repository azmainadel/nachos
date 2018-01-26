// progtest.cc
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"

#include "memorymanager.h"
#include "processtable.h"
#include "synchconsole.h"
#include "thread.h"

MemoryManager *memoryManager;
MemoryManager *swapMemoryManager;
unsigned char *swapBits;
Lock *memoryLock, *syscallLock;
ProcessTable *processTable;
int* timestamps;
SynchConsole *synchConsole;

Semaphore *synchReadAvail;
Semaphore *synchWriteDone;

static void SynchReadAvail(void* arg) { synchReadAvail->V(); }
static void SynchWriteDone(void* arg) { synchWriteDone->V(); }


class ExecuteOnce{
public:
    ExecuteOnce()
    {
        if(t == 0){
            memoryManager = new MemoryManager(NumPhysPages);
            swapMemoryManager = new MemoryManager(256);
            swapBits = new unsigned char[128 * 256];
            // swapBits = new unsigned char[128 * 1024]; //MATMULT
            memoryLock = new Lock("memory lock");
            syscallLock = new Lock("syscall lock");
            processTable = new ProcessTable(100);
            timestamps = new int[NumPhysPages];

            for(int i = 0; i < NumPhysPages; i++){
                timestamps[i] = 0;
            }

            synchConsole = new SynchConsole(NULL, NULL, SynchReadAvail, SynchWriteDone, 0);
            synchReadAvail = new Semaphore("synchReadAvail", 0);
            synchWriteDone = new Semaphore("synchWriteDone", 0);

            t++;
        }
    }
private:
    static int t;
};

int ExecuteOnce::t = 0;



//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------

void
//StartProcess(const char *filename)
StartProcess(void *arg)
{
    const char *filename = (const char *) arg;
    ExecuteOnce *executeOnce = new ExecuteOnce();

    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
    	printf("Unable to open file %s\n", filename);
    	return;
    }
    space = new AddrSpace(executable);
    currentThread->space = space;
    currentThread->id = processTable->Alloc( (void *) currentThread );

    // delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(false);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

void
StartMultipleProcess(int argc, char **argv)
{
    for(int i = 1; i < argc; ++i)
    {
        char *name = new char[100];
        sprintf(name, "multiple proces %d", i);
        Thread *newThread = new Thread (name);
        newThread->Fork(StartProcess, (void *)*(argv + i));
    }
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(void* arg) { readAvail->V(); }
static void WriteDone(void* arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void
ConsoleTest (const char *in, const char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);

    for (;;) {
    	readAvail->P();		// wait for character to arrive
    	ch = console->GetChar();
    	console->PutChar(ch);	// echo it!
    	writeDone->P() ;        // wait for write to finish
    	if (ch == 'q') return;  // if q, quit
    }
}
