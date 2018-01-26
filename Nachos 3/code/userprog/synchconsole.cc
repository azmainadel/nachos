#include "synchconsole.h"
#include "machine.h"

SynchConsole::SynchConsole(const char *readFile, const char *writeFile,
		VoidFunctionPtr readAvail, 
		VoidFunctionPtr writeDone, void* callArg)
{
	console = new Console(readFile, writeFile, readAvail, writeDone, 0);
	synchConsoleLock = new Lock("synch console lock");
}

SynchConsole::~SynchConsole()
{
	delete synchConsoleLock;
	delete console;
}

void
SynchConsole::Read(int str, int size, int processID)
{
	synchConsoleLock->Acquire();
	int nowSize = 0;
	int ch;
	while(nowSize < size)
	{
		synchReadAvail->P();
		ch = console->GetChar();
		if(ch != (int)'\n')
		{
			machine->WriteMem(str + nowSize, 1, ch);
		}
		else
		{
			ch = (int)'\0';
			machine->WriteMem(str + nowSize, 1, ch);
			synchConsoleLock->Release();
			return ;
		}
		//printf("%c\n", (char)ch);
		nowSize++;
	}
	synchConsoleLock->Release();
}

void
SynchConsole::Write(int str, int size, int processID)
{
	synchConsoleLock->Acquire();
	int nowSize = 0;
	int ch;
	while(nowSize < size)
	{
		machine->ReadMem(str + nowSize, 1, &ch);
		console->PutChar(ch);
		synchWriteDone->P();
		nowSize++;
	}
	console->PutChar((int)'\n');
	synchConsoleLock->Release();
}