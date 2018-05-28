#include "str.h"

void
strcpy_const2ram(char *dest, const rom char *src)
{
    while ((*dest++ = *src++) != '\0')
        ;
}

void
strcpy_ram2ram(char *dest, const char *src)
{
    while ((*dest++ = *src++) != '\0')
        ;
}

unsigned char
cstrncmp(const char *str1, const rom char *str2, unsigned char n)
{
    unsigned char i;

    for (i = 0; i < n; i++)
    {
        if (str1[i] != str2[i])
            return 1;
    }

    return 0;
}
