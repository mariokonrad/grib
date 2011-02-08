#ifndef __GET_BITS__H__
#define __GET_BITS__H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int get_bits(const unsigned char * buf, int * loc, size_t off, size_t bits);

#ifdef __cplusplus
}
#endif

#endif
