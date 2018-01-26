#include "copyright.h"
#include "table.h"
#include "synch.h"

Lock* tableLock = new Lock("tableLock");
// extern Table* pTable;

Table::Table(int size){
    array = new void*[size + 1];
    for(int i = 0; i < size; i++) array[i] = NULL;

    numberOfProcesses = 0;
    tableSize = size;
}

int Table::Alloc(void* object){
    tableLock->Acquire();

    int val = -1;
    for(int i = 0; i < tableSize; i++){
        if(array[i] == NULL){
            array[i] = object;
            numberOfProcesses++;
            val = i;
            break;
        }
    }

    tableLock->Release();
    return val;
}

void* Table::Get(int index){
    tableLock->Acquire();

    if(array[index] != NULL){
		tableLock->Release();
        return array[index];
    }

    tableLock->Release();
    return NULL;
}

void Table::Release(int index){
    tableLock->Acquire();

    array[index] = NULL;
    numberOfProcesses--;

    tableLock->Release();
}
