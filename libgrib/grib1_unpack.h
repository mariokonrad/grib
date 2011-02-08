#ifndef __GRIB1_UNPACK__H__
#define __GRIB1_UNPACK__H__

#include <grib1.h>

#ifdef __cplusplus
extern "C" {
#endif

int grib1_unpack(GRIBRecord * grib, int (*read_func)(void * buf, unsigned int len));

#ifdef __cplusplus
}
#endif

#endif
