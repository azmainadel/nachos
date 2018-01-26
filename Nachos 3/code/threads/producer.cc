#include "producer.h"

Producer::Producer(const char* debugName, GlobalBuffer* givenBuffer)
{
	name = debugName;
	buffer = givenBuffer;
}
Producer::~Producer()
{
	delete name;
	delete buffer;
}
void
Producer::Run()
{
	while(true)
	{
		buffer->AcquireLock(name);
		while(buffer->isQueueFull() == true)
		{
			buffer->MakeProducerWait();
		}
		buffer->Insert(Random() % 100 + 1, name);
		Delay(1);
		buffer->BroadcastToConsumer();
		buffer->ReleaseLock(name);

		//Delay(Random() % 5);
	}
}