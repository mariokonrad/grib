#ifndef __SET_BITS__H__
#define __SET_BITS__H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int set_bits(unsigned char *buf, int src, size_t off, size_t bits);

#ifdef __cplusplus
}
#endif

#endif
