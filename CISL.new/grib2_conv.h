#ifndef __GRIB2_CONV__H__
#define __GRIB2_CONV__H__

#include <grib2.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void grib2_packPDS(GRIBMessage * msg, int grid_number, unsigned char * grib1_buffer, size_t * offset);
void grib2_packGDS(GRIBMessage * msg, int grid_number, unsigned char * grib1_buffer, size_t * offset);
void grib2_packBMS(GRIBMessage * msg, int grid_number, unsigned char * grib1_buffer, size_t * offset, size_t num_points);
void grib2_packBDS(GRIBMessage * msg, int grid_number, unsigned char * grib1_buffer, size_t * offset, int * pvals, size_t num_to_pack, int pack_width);

#ifdef __cplusplus
}
#endif

#endif
