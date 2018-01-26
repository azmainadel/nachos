#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#ifndef BITMAP_H
#include "bitmap.h"
#endif

#ifndef SYNCH_H
#include "synch.h"
#endif

class MemoryManager{
public:
	MemoryManager(int numPages);
	~MemoryManager();
	
	int *processMap;
	TranslationEntry **entries;

	int AllocPage();
	void FreePage(int physPageNum);
	bool PageIsAllocated(int physPageNum);
	bool IsAnyPageFree();
	int NumFreePages();
	int AllocPage(int processNo, TranslationEntry *entry);
	int AllocByForce();
	int AllocByForceLRU();

private:
	BitMap *bitMap;
	Lock *lock;
	int numPages;
};

#endif
