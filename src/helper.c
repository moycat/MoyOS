/*
 * helper.c @ MoyOS
 *
 * Helper functions to get rid of C libraries.
 *
 */
#include "helper.h"

void* memcpy(void *destination, const void *source, uint32_t num)
{
    unsigned char *dst = destination, *src = source;
    while (num--) {
        *(dst++) = *(src++);
    }
    return destination;
}

void* memset(void *destination, int value, uint32_t n)
{
    const unsigned char v = (unsigned char)value;
    unsigned char *dst;
    for(dst = destination; n > 0; ++dst, --n)
        *dst = v;
    return destination;
}

char* strcpy(char *destination, const char *source)
{
    char *rt = destination;
    while (*source != '\0') {
        *(destination++) = *(source++);
    }
    *destination = '\0';
    return rt;
}
