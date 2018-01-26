#include "consumer.h"

Consumer::Consumer(const char* debugName, GlobalBuffer* givenBuffer)
{
	name = debugName;
	buffer = givenBuffer;
}
Consumer::~Consumer()
{
	delete name;
	delete buffer;
}
void
Consumer::Run()
{
	while(true)
	{
		buffer->AcquireLock(name);
		while(buffer->isQueueEmpty() == true)
		{
			buffer->MakeConsumerWait();
		}
		int now = buffer->Remove(name);
		Delay(1);
		buffer->BroadcastToProducer();
		buffer->ReleaseLock(name);

		//Delay(Random() % 5);
	}
}