
#include <stdio.h>

int main(int argc, char **argv)
{
    int i = 123, j = 15;
    
    printf("i = "); scanf("%d", &i);
    printf("j = "); scanf("%d", &j);
    
    printf("%d+%d=%d\n", i, j, i + j);
    printf("%d-%d=%d\n", i, j, i - j);
    printf("%d*%d=%d\n", i, j, i * j);
    printf("%d/%d=%d\n", i, j, i / j);
    printf("%d%%%d=%d\n", i, j, i % j);
    return 0;
}
