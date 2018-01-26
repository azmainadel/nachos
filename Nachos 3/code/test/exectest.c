/*
 * exectest.c
 *
 * Exec a simple program.  Return the result of Exec.
 */

#include "syscall.h"

int
main()
{
    int result;
	char str[4];
    result = Exec("../test/exittest");
	//void Write(char *buffer, int size, OpenFileId id);
	
	str[0] = 'a';
	str[1] = 'b';
	str[2] = 'c';
	Read(str, 3, ConsoleInput);
	Write(str, 3, ConsoleOutput);

	
  
    Exit(2);
}
