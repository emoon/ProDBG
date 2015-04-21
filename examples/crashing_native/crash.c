#include <stdio.h>

extern void callMe();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    int i;

    for (i = 0; i < 20; ++i)
        printf("%d\n", i);

    callMe();

    for (i = 0; i < 20; ++i)
        printf("%d\n", i);

    return 0;
}
