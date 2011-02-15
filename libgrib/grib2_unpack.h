#ifndef __GRIB2_UNPACK__H__
#define __GRIB2_UNPACK__H__

#include <grib2.h>

#ifdef __cplusplus
extern "C" {
#endif

int grib2_unpack(GRIBMessage * grib, int (*read_func)(void *, unsigned int, void *), void * ptr);

#ifdef __cplusplus
}
#endif

#endif
