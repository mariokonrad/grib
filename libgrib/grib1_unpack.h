#ifndef __GRIB1_UNPACK__H__
#define __GRIB1_UNPACK__H__

#include <grib1.h>

#ifdef __cplusplus
extern "C" {
#endif

void grib1_unpackGDS(GRIBRecord * grib);
void grib1_unpackBDS(GRIBRecord * grib);
void grib1_unpackPDS(GRIBRecord * grib);
int grib1_unpackIS(GRIBRecord * grib, int (*read_func)(void * buf, unsigned int len));
int grib1_unpack(GRIBRecord * grib, int (*read_func)(void * buf, unsigned int len));

#ifdef __cplusplus
}
#endif

#endif
