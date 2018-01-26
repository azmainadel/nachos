#include "copyright.h"
#include "memorymanager.h"
#include "synch.h"

Lock* memoryLock = new Lock("memoryLock");

MemoryManager::MemoryManager(int nPages){
    memorybitMap = new BitMap(nPages);
}

int MemoryManager::AllocPage(){
    memoryLock->Acquire();
    int page = memorybitMap->Find();
    memoryLock->Release();

    return page;
}

void MemoryManager::FreePage(int physPageNum){
    memoryLock->Acquire();
    memorybitMap->Clear(physPageNum);
    memoryLock->Release();

    return;
}

bool MemoryManager::PageIsAllocated(int physPageNum){
    memoryLock->Acquire();
    bool flag = memorybitMap->Test(physPageNum);
    memoryLock->Release();

    return flag;
}
