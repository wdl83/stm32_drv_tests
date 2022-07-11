#include "libc.h"

void bzero(void *addr, size_t size)
{
    while(size) ((char *)addr)[--size] =  UINT8_C(0);
}
