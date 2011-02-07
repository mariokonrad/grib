#include <stdio.h>
#include <grib2.h>
#include <grib2_conv.h>
#include <grib2_unpack.h>
#include <set_bits.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char ** argv)
{
	GRIBMessage grib_msg;
	FILE * fp;
	FILE * ofp;
	size_t nmsg = 0;
	size_t ngrid = 0;
	int status;
	int length;
	int max_length = 0;
	int num_points;
	int num_to_pack;
	int pack_width;
	int * pvals;
	int max_pack;
	unsigned char * grib1_buffer = NULL;
	unsigned char dum[3];
	char * head = "GRIB";
	char * tail = "7777";
	size_t n;
	size_t m;
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

	while ((status=grib2_unpack(fp,&grib_msg)) == 0) {
		nmsg++;
		for (n = 0; n < grib_msg.num_grids; n++) {
			/* calculate the octet length of the GRIB1 grid (minus the Indicator and End
			   Sections, which are both fixed in length */
			switch (grib_msg.md.pds_templ_num) {
				case 0:
				case 8:
					length=28;
					break;
				case 1:
				case 11:
					length=43;
					break;
				case 2:
				case 12:
					length=42;
					break;
				default:
					fprintf(stderr,"Unable to map Product Definition Template %d into GRIB1\n",grib_msg.md.pds_templ_num);
					return -1;
			}

			switch (grib_msg.md.gds_templ_num) {
				case 0:
					length+=32;
					num_points=grib_msg.md.nx*grib_msg.md.ny;
					break;
				case 30:
					length+=42;
					num_points=grib_msg.md.nx*grib_msg.md.ny;
					break;
				default:
					fprintf(stderr,"Unable to map Grid Definition Template %d into GRIB1\n",grib_msg.md.gds_templ_num);
					return -1;
			}

			if (grib_msg.grids[n].md.bitmap != NULL) {
				length+=6+(num_points+7)/8;
				num_to_pack=0;
				for (m=0; m < num_points; m++) {
					if (grib_msg.grids[n].md.bitmap[m] == 1) {
						num_to_pack++;
					}
				}
			} else {
				num_to_pack=num_points;
			}
			pvals=(int *)malloc(sizeof(int)*num_to_pack);
			max_pack=0;
			cnt=0;
			for (m=0; m < num_points; m++) {
				if (grib_msg.grids[n].gridpoints[m] != GRIB_MISSING_VALUE) {
					pvals[cnt]=lroundf((grib_msg.grids[n].gridpoints[m]-grib_msg.grids[n].md.R)*pow(10.,grib_msg.grids[n].md.D)/pow(2.,grib_msg.grids[n].md.E));
					if (pvals[cnt] > max_pack) {
						max_pack=pvals[cnt];
					}
					cnt++;
				}
			}
			pack_width=1;
			while (pow(2.,pack_width)-1 < max_pack) {
				pack_width++;
			}
			length+=11+(num_to_pack*pack_width+7)/8;
			/* allocate enough memory for the GRIB1 buffer */
			if (length > max_length) {
				if (grib1_buffer != NULL) {
					free(grib1_buffer);
				}
				grib1_buffer=(unsigned char *)malloc(length*sizeof(unsigned char));
				max_length=length;
			}
			offset=0;
			/* pack the Product Definition Section */
			grib2_packPDS(&grib_msg,n,grib1_buffer,&offset);
			/* pack the Grid Definition Section */
			grib2_packGDS(&grib_msg,n,grib1_buffer,&offset);
			/* pack the Bitmap Section, if it exists */
			if (grib_msg.grids[n].md.bitmap != NULL) {
				grib2_packBMS(&grib_msg,n,grib1_buffer,&offset,num_points);
			}
			/* pack the Binary Data Section */
			grib2_packBDS(&grib_msg,n,grib1_buffer,&offset,pvals,num_to_pack,pack_width);
			free(pvals);
			/* output the GRIB1 grid */
			fwrite(head,1,4,ofp);
			set_bits(dum,length+12,0,24);
			fwrite(dum,1,3,ofp);
			dum[0]=1;
			fwrite(dum,1,1,ofp);
			fwrite(grib1_buffer,1,length,ofp);
			fwrite(tail,1,4,ofp);
			ngrid++;
		}
	}
	if (status != -1) {
		printf("Read error after %d messages\n",nmsg);
	}
	printf("Number of GRIB1 grids written to output: %d\n",ngrid);
	fclose(fp);
	fclose(ofp);

	return 0;
}
