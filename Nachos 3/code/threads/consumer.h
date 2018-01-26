#ifndef CONSUMER_H
#define CONSUMER_H

#ifndef GLOBALBUFFER_H
#include "globalbuffer.h"
#endif

class Consumer{
	public:
		Consumer(const char* debugName, GlobalBuffer* givenBuffer);
		~Consumer();
		void Run();
	private:
		const char* name;
		GlobalBuffer* buffer;
};



#endif //CONSUMER_H