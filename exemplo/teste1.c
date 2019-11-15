#include <stdio.h>

#include "../include/t2fs.h"

int main()
{
    printf("Main\n");
    printf(" %d", sizeof(unsigned short int));
    logMbr();
    readSuper(0, 256);
    return 0;
}
