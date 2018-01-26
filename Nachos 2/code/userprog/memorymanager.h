#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "bitmap.h"

class MemoryManager{
private:
    BitMap* memorybitMap;

public:
    MemoryManager(int nPages);
    int AllocPage();
    void FreePage(int physPageNum);
    bool PageIsAllocated(int physPageNum);
};

#endif
