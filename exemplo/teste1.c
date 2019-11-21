#include <stdio.h>
#include <string.h>
#include "../include/t2fs.h"

int main()
{

    printf("Main\n");
    initT2FS();
    format2(0, 2);
    mount(0);
    opendir2();
    int i = 0;
    for (i = 0; i < 50; i++)
    {
        create2(itoa(i));
    }

    return 0;
}
