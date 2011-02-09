#ifndef __CONV_FLOAT__H__
#define __CONV_FLOAT__H__

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

double ibm2real(unsigned char * buf, size_t off);
int32_t ieee2ibm(double ieee);

#ifdef __cplusplus
}
#endif

#endif
