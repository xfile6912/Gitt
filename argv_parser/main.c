#include <stdio.h>
#include <string.h>
#include "argv_parser.h"



int main(void)
{
    char str[MAX_LENGTH];
    char argv[MAX_ARGC][MAX_LENGTH];
    int argc=0;
    int i;

    //get string
    scanf("%[^\n]", str);

    argc = parse_argv(argv, str);

    printf("string : %s\n", str);
    printf("argc : %d\n", argc);
    printf("argv : \n");
    for(i=0; i<argc; i++)
    {
        printf("    %d: %s\n", i, argv[i]);
    }
}