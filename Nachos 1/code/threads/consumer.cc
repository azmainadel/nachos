#include "copyright.h"
#include "system.h"
#include "consumer.h"
#include <stdlib.h>

Consumer::Consumer(char* name){
    this->name = name;
}

void Consumer::consume(void* v){
    while(1){
        lock->Acquire();

    if(itemCount == 0) tableEmpty->Wait(lock);

    for(int i = 0; i < (rand() % 500000 + 10000000); i++);

    int item = items->Remove();
    itemCount--;
    printf("Item: %d consumed by Consumer: %s\n", item, (char*) v);
    printf("Items on table: %d | Empty space: %d\n", itemCount, maxItems - itemCount);

    if(itemCount == maxItems - 1) tableFull->Signal(lock);

    lock->Release();
    for(int i = 0; i < (rand() % 500000 + 10000000); i++);
}
}
