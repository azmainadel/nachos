#include "memorymanager.h"
#include "stdlib.h"
#include "system.h"
#define MIN_STAMP 9999999

extern int* timestamps;

MemoryManager::MemoryManager(int numPages)
{
	this->numPages = numPages;
	bitMap = new BitMap(numPages);
	lock = new Lock("lock of memory manager");
	processMap = new int[numPages];
	entries = new TranslationEntry*[numPages];
}

MemoryManager::~MemoryManager()
{
	delete bitMap;
	delete lock;
	// delete processMap;
	// delete entries;
}

int MemoryManager::AllocPage()
{
	lock->Acquire();
	int ret = bitMap->Find();
	lock->Release();
	return ret;
}

int MemoryManager::AllocPage(int processNo, TranslationEntry *entry)
{
	lock->Acquire();
	int ret = bitMap->Find();

	if(ret != -1){
		processMap[ret] = processNo;
		entries[ret] = entry;
	}
	lock->Release();
	return ret;
}

int MemoryManager::AllocByForce(){
	lock->Acquire();

	int ret = rand() % numPages;
	entries[ret]->valid = false;

	lock->Release();
	return ret;
}

int MemoryManager::AllocByForceLRU(){
	lock->Acquire();

	// int minTimestamp = MIN_STAMP;
	int minTimestamp = timestamps[0];
	int ret = 0;

	// printf("%d\n", numPages);

	for(int i = 1; i < numPages; i++){
		if(timestamps[i] < minTimestamp /*&& entries[i]->dirty == false*/){
			minTimestamp = timestamps[i];
			ret = i;

			// printf("MIN: %d RET: %d\n", minTimestamp, ret);
		}
	}
	entries[ret]->valid = false;

	lock->Release();
	return ret;
}

void MemoryManager::FreePage(int physPageNum)
{
	lock->Acquire();
	bitMap->Clear(physPageNum);
	lock->Release();
}

bool
MemoryManager::PageIsAllocated(int physPageNum)
{
	lock->Acquire();
	bool ret = bitMap->Test(physPageNum);
	lock->Release();
	return ret;
}

bool
MemoryManager::IsAnyPageFree()
{
	lock->Acquire();
	bool ret;
	if(bitMap->NumClear() == 0)
		ret = false;
	else
		ret = true;
	lock->Release();
	return ret;
}

int
MemoryManager::NumFreePages()
{
	lock->Acquire();
	int ret = bitMap->NumClear();
	lock->Release();
	return ret;
}
