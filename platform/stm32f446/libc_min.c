/*
 * libc_min.c -- WRITTEN BY ME.
 * We link with -nostdlib, but GCC may still emit calls to memcpy/memset
 * (e.g. when zero-initialising a struct). Provide the minimum.
 */
#include <stddef.h>

void *memset(void *dst, int c, size_t n)
{
    unsigned char *d = dst;
    while (n--) { *d++ = (unsigned char)c; }
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) { *d++ = *s++; }
    return dst;
}
