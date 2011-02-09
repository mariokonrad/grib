#ifndef __CONV_FLOAT__H__
#define __CONV_FLOAT__H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

double ibm2real(const unsigned char * buf, unsigned int off);
int32_t ieee2ibm(double ieee);

#ifdef __cplusplus
}
#endif

#endif
