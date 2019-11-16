#include <stdio.h>

#include "../include/t2fs.h"

int main()
{
    printf("Main\n");
    logMbr();
    format2(0, 2);
    readSuper(0);
    return 0;
}
