#include <stdarg.h>

#include "libc.h"

void *memcpy(void *dst, const void *src, size_t size)
{
    while(size--) ((uint8_t *)dst)[size] =  ((const uint8_t *)src)[size];
    return dst;
}

void *memset(void *dst, int c, size_t size)
{
    while(size) ((uint8_t *)dst)[--size] = UINT8_C(c);
    return dst;
}

void bzero(void *dst, size_t size)
{
    while(size) ((uint8_t *)dst)[--size] =  UINT8_C(0);
}
