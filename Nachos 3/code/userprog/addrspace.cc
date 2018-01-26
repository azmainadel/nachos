// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "synch.h"
#include "memorymanager.h"
#include "machine.h"
#include <iostream>
using namespace std;

extern MemoryManager *memoryManager;
extern MemoryManager *swapMemoryManager;
extern Lock *memoryLock;

extern unsigned char *swapBits;

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
	noffH = new NoffHeader();
    unsigned int i, j, size;
	this->executable = executable;

    this->executable->ReadAt((char *)noffH, sizeof(*noffH), 0);
    if ((noffH->noffMagic != NOFFMAGIC) &&
		(WordToHost(noffH->noffMagic) == NOFFMAGIC))
    	SwapHeader(noffH);
    ASSERT(noffH->noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH->code.size + noffH->initData.size + noffH->uninitData.size
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    // ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
					numPages, size);
// first, set up the translation
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
    	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
    	//pageTable[i].physicalPage = i;
        pageTable[i].physicalPage = -1;
        // else
        // {
        //     for(j = 0; j < i; ++j)
        //         memoryManager->FreePage(pageTable[j].physicalPage);
        //     ASSERT(false);
        // }
		pageTable[i].swapIndex = -1;
    	pageTable[i].valid = false;
    	pageTable[i].use = false;
    	pageTable[i].dirty = false;
    	pageTable[i].readOnly = false;  // if the code segment was entirely on
                    					// a separate page, we could set its
                    					// pages to be read-only
    }

    // zero out the entire address space, to zero the unitialized data segment
    // and the stack segment
    //bzero(machine->mainMemory, size);

    // memoryLock->Acquire();
    // for(i = 0; i < numPages; ++i)
    // {
    //     bzero(&machine->mainMemory[pageTable[i].physicalPage * PageSize], PageSize);
    // }
	//
	//
    // // then, copy in the code and data segments into memory
    // unsigned int numPagesForCode = divRoundUp(noffH.code.size, PageSize);
    // DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",
	// 	noffH.code.virtualAddr, noffH.code.size);
    // for(i = 0; i < numPagesForCode; ++i)
    // {
    //     executable->ReadAt(&(machine->mainMemory[ pageTable[i].physicalPage * PageSize ]),
    //                         PageSize,
    //                         noffH.code.inFileAddr + i * PageSize);
    // }
	//
    // unsigned int numPagesForData = divRoundUp(noffH.initData.size, PageSize);
	//
    // DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",
    //     noffH.initData.virtualAddr, noffH.initData.size);
    // for(j = numPagesForCode; j < numPagesForCode + numPagesForData; ++j)
    // {
    //     executable->ReadAt(&(machine->mainMemory[ pageTable[i].physicalPage * PageSize ]),
    //                         PageSize,
    //                         noffH.initData.inFileAddr + (j - numPagesForCode) * PageSize);
    // }
    // memoryLock->Release();

}

AddrSpace::~AddrSpace()
{
   // delete pageTable;
   for(unsigned int i = 0; i < machine->pageTableSize; i++){
	   if(machine->pageTable[i].valid) memoryManager->FreePage(machine->pageTable[i].physicalPage);
   }
   delete executable;
   delete noffH;
   delete pageTable;
}

int AddrSpace::loadIntoFreePage(int address, int physicalPageNumber){
	printf("Address: %d, physicalPage: %d\n", address, physicalPageNumber);

	int virtualPageNumber = address / PageSize;
	pageTable[virtualPageNumber].physicalPage = physicalPageNumber;
	pageTable[virtualPageNumber].valid = true;

    if (isSwapPageExists(virtualPageNumber) == true){
		loadFromSwapSpace(virtualPageNumber);

		printf("-----Swapping Pages-----\n");
	}

	else if(address >= noffH->code.virtualAddr && address < noffH->code.virtualAddr + noffH->code.size){

		int codeOffset = (address - noffH->code.virtualAddr) / PageSize;
		int codeSize = min(noffH->code.size - codeOffset * PageSize, PageSize);

		executable->ReadAt(&(machine->mainMemory[physicalPageNumber * PageSize]),
			codeSize, noffH->code.inFileAddr + codeOffset * PageSize);

		if(codeSize < PageSize){
			int dataSize = PageSize - codeSize;
			int noffDataSize = noffH->initData.size;

			if(noffDataSize > dataSize){
			executable->ReadAt(&(machine->mainMemory[ physicalPageNumber * PageSize + codeSize]),
				dataSize, noffH->initData.inFileAddr);
			}
			else{
				executable->ReadAt(&(machine->mainMemory[ physicalPageNumber * PageSize + codeSize]),
					noffDataSize, noffH->initData.inFileAddr);

				bzero(&(machine->mainMemory[physicalPageNumber * PageSize + codeSize + noffH->initData.size]), dataSize - noffDataSize);
			}
		}

	}

	else if(address >= noffH->initData.virtualAddr && address < noffH->initData.virtualAddr + noffH->initData.size){

		int dataOffset = virtualPageNumber * PageSize - noffH->initData.virtualAddr;
		int dataSize = min(noffH->initData.size  + noffH->initData.virtualAddr - virtualPageNumber * PageSize, PageSize);

		executable->ReadAt(&(machine->mainMemory[physicalPageNumber * PageSize]),
			dataSize, noffH->initData.inFileAddr + dataOffset);

		if(dataSize < PageSize){
			int uninitDataSize = PageSize - dataSize;
			bzero(&(machine->mainMemory[physicalPageNumber * PageSize + dataSize]), uninitDataSize);
		}
	}

	else{
		bzero(&machine->mainMemory[physicalPageNumber * PageSize], PageSize);
	}

	return 0;
}


//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

void AddrSpace::loadFromSwapSpace(int virtualPageNumber){
	for(int i = 0; i < PageSize; i++){
		machine->mainMemory[pageTable[virtualPageNumber].physicalPage * PageSize + i] = swapBits[pageTable[virtualPageNumber].swapIndex * PageSize + i];
	}
}

void AddrSpace::saveIntoSwapSpace(int virtualPageNumber){
	if(pageTable[virtualPageNumber].swapIndex == -1) pageTable[virtualPageNumber].swapIndex = swapMemoryManager->AllocPage();

	for(int i = 0; i < PageSize; i++){
		 swapBits[pageTable[virtualPageNumber].swapIndex * PageSize + i] = machine->mainMemory[pageTable[virtualPageNumber].physicalPage * PageSize + i];
	}
}

bool AddrSpace::isSwapPageExists(int virtualPageNumber){
	if(pageTable[virtualPageNumber].swapIndex == -1) return false;
	return true;
}
