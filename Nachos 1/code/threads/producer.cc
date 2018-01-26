#include "copyright.h"
#include "system.h"
#include "producer.h"
#include <stdlib.h>

extern Lock* lock = new Lock("lock");
extern Condition* tableEmpty = new Condition("tableEmpty");
extern Condition* tableFull = new Condition("tableFull");
extern List<int>* items = new List<int>;
extern int itemCount = 0;
extern const int maxItems = 20;

Producer::Producer(char* name){
    this->name = name;
}

void Producer::produce(void* v){
    while(1){
        lock->Acquire();

    if(itemCount == maxItems) tableFull->Wait(lock);

    for(int i = 0; i < (rand() % 500000 + 10000000); i++);


    int item = rand() % 1000;
    items->Append(item);
    itemCount++;
    printf("Item: %d produced by Producer: %s\n", item, (char*) v);
    printf("Items on table: %d | Empty space: %d\n", itemCount, maxItems - itemCount);

    if(itemCount == 1) tableEmpty->Signal(lock);

    lock->Release();
    for(int i = 0; i < (rand() % 500000 + 10000000); i++);
}
}
