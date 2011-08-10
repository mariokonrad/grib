#include <cstdio>
#include <cstring>
#include <grib2_unpack.h>

static int read_func(void * buf, unsigned int len, void * ptr)
{
	if (buf == NULL) return 0;
	if (ptr == NULL) return 0;
	FILE * file = static_cast<FILE *>(ptr);
	return fread(buf, 1, len, file);
}

int main(int, char ** argv)
{
	GRIBMessage grib;
	memset(&grib, 0, sizeof(grib));

	FILE * file = fopen(argv[1], "r");
	for (;;) {
		int rc = grib2_unpack(&grib, read_func, file);
		if (rc < 0) break;

		for (int i = 0; i < grib.num_grids; ++i) {
			GRIB2Grid & grid = grib.grids[i];

/*
			printf("GRID %d\n", i);
			printf("\tearth_shape: %d\n", grid.md.earth_shape);
			printf("\tnx/ny: %d / %d\n", grid.md.nx, grid.md.ny);
			printf("\tslat: %f\n", grid.md.slat);
			printf("\tslon: %f\n", grid.md.slon);
			printf("\telat: %f\n", grid.md.lats.elat);
			printf("\telon: %f\n", grid.md.lons.elon);
*/

			if (grid.gridpoints == NULL) {
				printf("\nNO GRIDPOINTS\n");
				return -1;
			}

			if (grid.md.pds_templ_num == 0 && grid.md.param_cat == 2) {
				if (grid.md.param_num == 2) { // UGRDA
					for (int y = 0; y < grid.md.ny; ++y) {
						for (int x = 0; x < grid.md.nx; ++x) {
							double lat = (grid.md.lats.elat - grid.md.slat) / (grid.md.ny - 1) * y + grid.md.slat;
							double lon = (grid.md.lons.elon - grid.md.slon) / (grid.md.nx - 1) * x + grid.md.slon;
							printf("UGRD:%1.0f,%1.0f,%1.2f\n", lon, lat, grid.gridpoints[y * grid.md.nx + x]);
						}
					}
				} else if (grid.md.param_num == 3) { // VGRD
					for (int y = 0; y < grid.md.ny; ++y) {
						for (int x = 0; x < grid.md.nx; ++x) {
							double lat = (grid.md.lats.elat - grid.md.slat) / (grid.md.ny - 1) * y + grid.md.slat;
							double lon = (grid.md.lons.elon - grid.md.slon) / (grid.md.nx - 1) * x + grid.md.slon;
							printf("VGRD:%1.0f,%1.0f,%1.2f\n", lon, lat, grid.gridpoints[y * grid.md.nx + x]);
						}
					}
				}
			}

		}
	}
	fclose(file);

	return 0;
}

