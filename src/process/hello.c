#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("Hello, world!\n");

    char *progname = argc ? argv[0] : "hello";
    printf("This is the %s program speaking!\n", progname);

    return 0;
}

