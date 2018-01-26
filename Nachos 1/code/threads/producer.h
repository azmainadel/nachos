#ifndef PRODUCER_H
#define PRODUCER_H

#include "synch.h"

extern Lock* lock;
extern Condition* tableEmpty;
extern Condition* tableFull;
extern List<int>* items;
extern int itemCount;
extern const int maxItems;

class Producer{
private:
    char* name;

public:
    Producer(char* name);

    char* getName() { return name;}
    static void produce(void* v);
};

#endif
