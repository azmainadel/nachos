#ifndef TABLE_H
#define TABLE_H

#include "bitmap.h"

class Table{
private:
    void** array;
    int numberOfProcesses;
    int tableSize;

public:
    Table(int size);
    int Alloc(void* object);
    void* Get(int index);
    void Release(int index);
};

#endif
