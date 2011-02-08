#ifndef __GRIB2_UNPACK__H__
#define __GRIB2_UNPACK__H__

#include <grib2.h>

#ifdef __cplusplus
extern "C" {
#endif

int grib2_unpack(GRIBMessage * grib, int (*read_func)(void * buf, unsigned int len));

#ifdef __cplusplus
}
#endif

#endif
