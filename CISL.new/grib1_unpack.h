#ifndef __GRIB1_UNPACK__H__
#define __GRIB1_UNPACK__H__

#include <stdio.h>
#include <grib1.h>

#ifdef __cplusplus
extern "C" {
#endif

void grib1_unpackGDS(GRIBRecord * grib);
void grib1_unpackBDS(GRIBRecord * grib);
void grib1_unpackPDS(GRIBRecord * grib);
int grib1_unpackIS(FILE * fp, GRIBRecord * grib);
int grib1_unpack(FILE * fp, GRIBRecord * grib);

#ifdef __cplusplus
}
#endif

#endif
