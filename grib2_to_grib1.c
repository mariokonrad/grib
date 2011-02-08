#include <stdio.h>
#include <grib2.h>
#include <grib2_conv.h>
#include <grib2_unpack.h>
#include <grib1_write.h>
#include <set_bits.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

static FILE * fp = NULL;
static FILE * ofp = NULL;

static int read_func(void * buf, unsigned int len)
{
	return fp == NULL
		? 0
		: fread(buf, 1, len, fp);
}

static int write_func(const void * buf, unsigned int len)
{
	return ofp == NULL
		? 0
		: fwrite(buf, 1, len, ofp);
}

int main(int argc, char ** argv)
{
	GRIBMessage grib_msg;
	size_t nmsg = 0;
	size_t ngrid = 0;
	int status;
	int length;
	int max_length = 0;
	int num_points;
	int num_to_pack;
	int pack_width;
	int * pvals = NULL;
	int max_pack;
	unsigned char * grib1_buffer = NULL;
	int n;
	int m;
	size_t offset;
	size_t cnt;

	grib_msg.buffer = NULL;
	grib_msg.grids = NULL;

	if (argc != 3) {
		fprintf(stderr, "usage: %s GRIB2_file_name GRIB1_file_name\n", argv[0]);
		return -1;
	}

	fp = fopen(argv[1], "rb");
	ofp = fopen(argv[2], "wb");

	while ((status = grib2_unpack(&grib_msg, read_func)) == 0) {
		nmsg++;
		for (n = 0; n < grib_msg.num_grids; n++) {
			/* calculate the octet length of the GRIB1 grid (minus the Indicator and End
			   Sections, which are both fixed in length */
			switch (grib_msg.md.pds_templ_num) {
				case 0:
				case 8:
					length = 28;
					break;
				case 1:
				case 11:
					length = 43;
					break;
				case 2:
				case 12:
					length = 42;
					break;
				default:
					fprintf(stderr,"Unable to map Product Definition Template %d into GRIB1\n", grib_msg.md.pds_templ_num);
					return -1;
			}

			switch (grib_msg.md.gds_templ_num) {
				case 0:
					length += 32;
					num_points = grib_msg.md.nx * grib_msg.md.ny;
					break;
				case 30:
					length += 42;
					num_points = grib_msg.md.nx * grib_msg.md.ny;
					break;
				default:
					fprintf(stderr,"Unable to map Grid Definition Template %d into GRIB1\n", grib_msg.md.gds_templ_num);
					return -1;
			}

			if (grib_msg.grids[n].md.bitmap != NULL) {
				length += 6 + (num_points + 7) / 8;
				num_to_pack = 0;
				for (m = 0; m < num_points; m++) {
					if (grib_msg.grids[n].md.bitmap[m] == 1) {
						num_to_pack++;
					}
				}
			} else {
				num_to_pack = num_points;
			}

			pvals = (int *)malloc(sizeof(int) * num_to_pack);
			max_pack = 0;
			cnt = 0;
			for (m = 0; m < num_points; m++) {
				if (grib_msg.grids[n].gridpoints[m] != GRIB_MISSING_VALUE) {
					pvals[cnt] = lroundf((grib_msg.grids[n].gridpoints[m] - grib_msg.grids[n].md.R) * pow(10.0, grib_msg.grids[n].md.D) / pow(2.0, grib_msg.grids[n].md.E));
					if (pvals[cnt] > max_pack) {
						max_pack = pvals[cnt];
					}
					cnt++;
				}
			}
			pack_width = 1;
			while (pow(2.0, pack_width) - 1 < max_pack) {
				pack_width++;
			}
			length += 11 + (num_to_pack * pack_width + 7) / 8;

			/* allocate enough memory for the GRIB1 buffer */
			if (length > max_length) {
				if (grib1_buffer != NULL) {
					free(grib1_buffer);
				}
				grib1_buffer = (unsigned char *)malloc(length * sizeof(unsigned char));
				max_length = length;
			}
			offset = 0;

			/* pack the Product Definition Section */
			if (grib2_packPDS(&grib_msg, n, grib1_buffer, &offset) != 0) {
				assert(0);
			}

			/* pack the Grid Definition Section */
			if (grib2_packGDS(&grib_msg, n, grib1_buffer, &offset) != 0) {
				assert(0);
			}

			/* pack the Bitmap Section, if it exists */
			if (grib_msg.grids[n].md.bitmap != NULL) {
				if (grib2_packBMS(&grib_msg, n, grib1_buffer, &offset, num_points) != 0) {
					assert(0);
				}
			}

			/* pack the Binary Data Section */
			if (grib2_packBDS(&grib_msg, n, grib1_buffer, &offset, pvals, num_to_pack, pack_width) != 0) {
				assert(0);
			}

			free(pvals);

			/* output the GRIB1 grid */
			if (grib1_write_raw(grib1_buffer, length, &write_func) != 0) {
				fprintf(stderr, "Cannot write GRIB1 file\n");
				return -1;
			}
			ngrid++;
		}
	}
	if (status != -1) {
		printf("Read error after %d messages\n", nmsg);
	}
	printf("Number of GRIB1 grids written to output: %d\n", ngrid);
	fclose(fp);
	fclose(ofp);

	return 0;
}

