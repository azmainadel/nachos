#ifndef PAGETABLE_H
#define PAGETABLE_H

#ifndef BITMAP_H
#include "bitmap.h"
#endif

#ifndef SYNCH_H
#include "synch.h"
#endif


class Process{
public:
	Process();
	~Process();
	void Set(int threadId, void *object);
	int GetID();
	void *GetProcess();
private:
	int id;
	void *process;
};

class ProcessTable{
public:
	ProcessTable(int size);
	~ProcessTable();
	int Alloc(void *object);
	void *Get(int index);
	void Release(int index);
	int nowSize, maxSize;
private:
	Lock *processTableLock;
	BitMap *bitMap;
	Process **processes;
};

#endif