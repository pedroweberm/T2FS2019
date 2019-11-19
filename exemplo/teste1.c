#include <stdio.h>
#include <string.h>
#include "../include/t2fs.h"

int main()
{

    DIRENT2 dentry;
    strcpy(dentry.name, "Salzinho\0");
    dentry.TypeVal = 1;
    dentry.inodeNumber = 5;

    printf("Main\n");
    initT2FS();
//    logMbr();
    format2(0, 2);
    mount(0);
    opendir2();
    readdir2(&dentry);
//    readSuper(0);
    return 0;
}
