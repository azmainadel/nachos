#ifndef GLOBALBUFFER_H
#define GLOBALBUFFER_H

#ifndef SYNCH_H
#include "synch.h"
#endif

#ifndef LIST_H
#include "list.h"
#endif

#ifndef THREAD_H
#include "thread.h"
#endif

class GlobalBuffer{
	public:

		GlobalBuffer(const char* debugName, int givenSize);
		~GlobalBuffer();

		void AcquireLock(const char* byWho);
		void ReleaseLock(const char* byWho);

		void MakeProducerWait();
		void SignalToProducer();
		void BroadcastToProducer();

		void MakeConsumerWait();
		void SignalToConsumer();
		void BroadcastToConsumer();

		void Insert(int item, const char* byWho);
		int Remove(const char* byWho);
		bool isQueueEmpty();
		bool isQueueFull();

	private:

		const char* name;

		List<int>* queue;
		int maxSize;
		int size;

		Lock* lock;

		Condition* notEmpty;	//for consumer
		Condition* notFull;		//for producer
};

#endif //GLOBALBUFFER_H