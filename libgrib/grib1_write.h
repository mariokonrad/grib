#ifndef __GRIB1_WRITE__H__
#define __GRIB1_WRITE__H__

#include <grib1.h>

#ifdef __cplusplus
extern "C" {
#endif

int grib1_write_raw(unsigned char * buf, unsigned int len, int (write_func)(const void *, unsigned int));
int grib1_write(GRIBRecord * grib, int (write_func)(const void *, unsigned int));

#ifdef __cplusplus
}
#endif

#endif
