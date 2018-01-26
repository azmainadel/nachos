#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "copyright.h"
#include "system.h"
#include "machine.h"

#ifndef SYNCH_H
#include "synch.h"
#endif

#ifndef CONSOLE_H
#include "console.h"
#endif


extern Semaphore *synchReadAvail;
extern Semaphore *synchWriteDone;

class SynchConsole{
public:
	SynchConsole(const char *readFile, const char *writeFile,
		VoidFunctionPtr readAvail, 
		VoidFunctionPtr writeDone, void* callArg);
	~SynchConsole();
	void Read(int str, int size, int processID);
	void Write(int str, int size, int processID);
private:
	Lock *synchConsoleLock;
	Console *console;
};

#endif