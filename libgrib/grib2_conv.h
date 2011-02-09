#ifndef __GRIB2_CONV__H__
#define __GRIB2_CONV__H__

#include <grib2.h>
#include <bits.h>

#ifdef __cplusplus
extern "C" {
#endif

int grib2_to_grib1_packPDS(GRIBMessage * msg, int grid_number, Buffer * grib1);
int grib2_to_grib1_packGDS(GRIBMessage * msg, int grid_number, Buffer * grib1);
int grib2_to_grib1_packBMS(GRIBMessage * msg, int grid_number, Buffer * grib1, unsigned int num_points);
int grib2_to_grib1_packBDS(GRIBMessage * msg, int grid_number, Buffer * grib1, int * pvals, size_t num_to_pack, int pack_width);

#ifdef __cplusplus
}
#endif

#endif
