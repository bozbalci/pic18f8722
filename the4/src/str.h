#ifndef STR_H
#define STR_H

void
strcpy_const2ram(char *dest, const rom char *src);

void
strcpy_ram2ram(char *dest, const char *src);

unsigned char
cstrncmp(const char *str1, const rom char *str2, unsigned char n);

#endif // STR_H
