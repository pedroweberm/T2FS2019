#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/t2fs.h"

int main()
{
    printf("Main\n");
    initT2FS();
    format2(0, 2);
    mount(0);
    opendir2();
    int i = 0;

    create2("a");
    create2("aa");
    create2("aaa");
    create2("aaaa");
    create2("aaaaa");
    create2("aaaaaa");
    create2("aaaaaaa");
    create2("aaaaaaaa");
    create2("aaaaaaaaa");
    create2("aaaaaaaaaa");
    create2("aaaaaaaaaaa");
    create2("aaaaaaaaaaaa");
    create2("aaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaaaaaaaaaaaaa");
    create2("aaaaaaaaaaaaaaaaaaaaaaaab");
    create2("aaaaaaaaaaaaaaaaaaaaaaaac");
    create2("aaaaaaaaaaaaaaaaaaaaaaaad");
    create2("aaaaaaaaaaaaaaaaaaaaaaaae");
    create2("aaaaaaaaaaaaaaaaaaaaaaaaf");
    create2("aaaaaaaaaaaaaaaaaaaaaaaag");
    create2("aaaaaaaaaaaaaaaaaaaaaaaah");


    return 0;
}
