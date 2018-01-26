#ifndef PRODUCER_H
#define PRODUCER_H

#ifndef GLOBALBUFFER_H
#include "globalbuffer.h"
#endif

class Producer{
	public:
		Producer(const char* debugName, GlobalBuffer* givenBuffer);
		~Producer();
		void Run();
	private:
		const char* name;
		GlobalBuffer* buffer;
};

#endif //PRODUCER_H