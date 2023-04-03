#pragma once

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dst, const void *src, size_t size);
void *memset(void *dst, int c, size_t size);
void bzero(void *dst, size_t size);
