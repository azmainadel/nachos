#include "processtable.h"

Process::Process()
{

}

Process::~Process()
{
	
}

void
Process::Set(int threadId, void *object)
{
	id = threadId;
	process = object;
}

int
Process::GetID()
{
	return id;
}

void *
Process::GetProcess()
{
	return process;
}

ProcessTable::ProcessTable(int size)
{
	maxSize = size;
	nowSize = 0;
	processTableLock = new Lock("process table lock");
	bitMap = new BitMap(size);
	processes = new Process*[size];
	for(int i = 0; i < size; ++i)
		processes[i] = new Process();
}

ProcessTable::~ProcessTable()
{
	delete processTableLock;
	delete bitMap;
	for(int i = 0; i < maxSize; ++i)
		delete processes[i];
	delete[] processes;
}

int
ProcessTable::Alloc(void *object)
{
	if(bitMap->NumClear() == 0)
		return -1;
	processTableLock->Acquire();
	processes[nowSize] = new Process();
	int temp = bitMap->Find();
	processes[nowSize]->Set(temp, object);
	nowSize++;
	processTableLock->Release();
	return temp;
}

void *
ProcessTable::Get(int index)
{
	if(bitMap->Test(index) == false)
		return NULL;
	processTableLock->Acquire();
	void *temp = processes[index]->GetProcess();
	processTableLock->Release();
	return temp;
}

void
ProcessTable::Release(int index)
{
	processTableLock->Acquire();
	if(bitMap->Test(index) == true)
	{
		bitMap->Clear(index);
		nowSize--;
	}
	processTableLock->Release();
}