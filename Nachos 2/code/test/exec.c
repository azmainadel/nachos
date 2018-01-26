#include "syscall.h"

int main()
{
    char c[4];

    // Exec("../test/exit");
    Read(c, 6, ConsoleInput);
    Write(c, 6, ConsoleInput);
}
