#include "globalbuffer.h"
GlobalBuffer::GlobalBuffer(const char* debugName, int givenSize)
{
	name = debugName;

	maxSize = givenSize;
	size = 0;
	queue = new List<int>;

	lock = new Lock(debugName);

	notEmpty = new Condition(debugName, lock);
	notFull = new Condition(debugName, lock);
}
GlobalBuffer::~GlobalBuffer()
{
	delete name;
	delete queue;//[] deya lagte pare
	delete notEmpty;
	delete notFull;
}
void
GlobalBuffer::AcquireLock(const char* byWho)
{
	lock->Acquire();
}
void
GlobalBuffer::ReleaseLock(const char* byWho)
{
	lock->Release();
}
void
GlobalBuffer::MakeProducerWait()
{
	notFull->Wait();
}
void
GlobalBuffer::SignalToProducer()
{
	notFull->Signal();
}
void
GlobalBuffer::BroadcastToProducer()
{
	notFull->Broadcast();
}
void
GlobalBuffer::MakeConsumerWait()
{
	notEmpty->Wait();
}
void
GlobalBuffer::SignalToConsumer()
{
	notEmpty->Signal();
}
void
GlobalBuffer::BroadcastToConsumer()
{
	notEmpty->Broadcast();
}
void
GlobalBuffer::Insert(int item, const char* byWho)
{
	queue->Append(item);
	size++;
	printf("%s put something, size is %d\n", byWho, size);
}
int
GlobalBuffer::Remove(const char* byWho)
{
	size--;
	printf("%s removed something, size is %d\n", byWho, size);
	return queue->Remove();
}
bool
GlobalBuffer::isQueueEmpty()
{
	return size == 0;
}
bool
GlobalBuffer::isQueueFull()
{
	return size == maxSize;
}