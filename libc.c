#include "libc.h"

void bzero(void *addr, size_t size)
{
    while(size) ((char *)addr)[--size] =  UINT8_C(0);
}

void *memcpy(void *dst, const void *src, size_t size)
{
    while(size--) ((char *)dst)[size] =  ((const char *)src)[size];
    return dst;
}
