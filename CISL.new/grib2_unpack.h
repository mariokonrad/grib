#ifndef __GRIB2_UNPACK__H__
#define __GRIB2_UNPACK__H__

#include <grib2.h>

#ifdef __cplusplus
extern "C" {
#endif

void grib2_unpackIDS(GRIBMessage * grib_msg);
void grib2_unpackLUS(GRIBMessage * grib);
void grib2_unpackGDS(GRIBMessage * grib_msg);
void grib2_unpackPDS(GRIBMessage * grib_msg);
void grib2_unpackDRS(GRIBMessage * grib_msg);
void grib2_unpackBMS(GRIBMessage * grib_msg);
void grib2_unpackDS(GRIBMessage * grib_msg, int grid_num);
int grib2_unpackIS(GRIBMessage * grib_msg, int (*read_func)(void * buf, unsigned int len));
int grib2_unpack(GRIBMessage * grib, int (*read_func)(void * buf, unsigned int len));

#ifdef __cplusplus
}
#endif

#endif
