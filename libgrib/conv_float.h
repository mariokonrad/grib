#ifndef __CONV_FLOAT__H__
#define __CONV_FLOAT__H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

double ibm2real(unsigned char * buf, size_t off);
int ieee2ibm(double ieee);

#ifdef __cplusplus
}
#endif

#endif
