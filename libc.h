#pragma once

#include <stddef.h>
#include <stdint.h>

void bzero(void *addr, size_t size);
void *memcpy(void *dst, const void *src, size_t size);
