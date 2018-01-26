#ifndef CONSUMER_H
#define CONSUMER_H

#include "synch.h"

extern Lock* lock;
extern Condition* tableEmpty;
extern Condition* tableFull;
extern List<int>* items;
extern int itemCount;
extern const int maxItems;

class Consumer{
private:
    char* name;

public:
    Consumer(char* name);

    char* getName() { return name;}
    static void consume(void* v);
};

#endif
