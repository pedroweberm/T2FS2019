#include <stdio.h>
#include <string.h>
#include "../include/t2fs.h"

int main()
{

    printf("Main\n");
    initT2FS();
//    logMbr();
    format2(0, 2);
    mount(0);
    opendir2();
//    readSuper(0);
    return 0;
}
