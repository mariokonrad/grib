#include <grib2_unpack.h>
#include <get_bits.h>
#include <jasper/jasper.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define UNUSED_ARG(a) (void)(a)

static int dec_jpeg2000(char * injpc, int bufsize, int * outfld) /* {{{ */
{
	int ier;
	int i;
	int j;
	int k;
	jas_image_t * image = NULL;
	jas_stream_t * jpcstream = NULL;
	jas_image_cmpt_t * pcmpt = NULL;
	char * opts = NULL;
	jas_matrix_t * data = NULL;

	/*
	jas_init();
	*/

	ier = 0;

	/*   
	Create jas_stream_t containing input JPEG200 codestream in memory.
	*/       

	jpcstream = jas_stream_memopen(injpc, bufsize);

	/*   
	Decode JPEG200 codestream into jas_image_t structure.
	*/       
	image = jpc_decode(jpcstream, opts);
	if (image == 0) {
		printf(" jpc_decode return = %d \n", ier);
		return -3;
	}

	pcmpt = image->cmpts_[0];

	/*
	Expecting jpeg2000 image to be grayscale only.
	No color components.
	*/
	if (image->numcmpts_ != 1 ) {
		printf("dec_jpeg2000: Found color image. Grayscale expected.\n");
		return -5;
	}

	/*
	Create a data matrix of grayscale image values decoded from
	the jpeg2000 codestream.
	*/
	data = jas_matrix_create(jas_image_height(image), jas_image_width(image));
	jas_image_readcmpt(image, 0, 0, 0, jas_image_width(image), jas_image_height(image), data);

	/*
	Copy data matrix to output integer array.
	*/
	k = 0;
	for (i = 0;i < pcmpt->height_; i++) {
		for (j = 0;j < pcmpt->width_; j++) {
			outfld[k++] = data->rows_[i][j];
		}
	}

	/*
	Clean up JasPer work structures.
	*/
	jas_matrix_destroy(data);
	ier = jas_stream_close(jpcstream);
	jas_image_destroy(image);

	return 0;
} /* }}} */

int grib2_unpackIS(GRIBMessage * grib_msg, int (*read_func)(void * buf, unsigned int len)) /* {{{ */
{
	unsigned char temp[16];
	int status;
	int n;
	size_t num;

	if (read_func == NULL) {
		return -1;
	}

	if (grib_msg->buffer != NULL) {
		free(grib_msg->buffer);
		grib_msg->buffer = NULL;
	} else {
		grib_msg->grids = NULL;
		grib_msg->md.stat_proc.proc_code = NULL;
	}
	if (grib_msg->grids != NULL) {
		for (n = 0; n < grib_msg->num_grids; n++) {
			if (grib_msg->grids[n].md.bitmap != NULL) {
				free(grib_msg->grids[n].md.bitmap);
				grib_msg->grids[n].md.bitmap = NULL;
			}
			free(grib_msg->grids[n].gridpoints);
		}
		free(grib_msg->grids);
		grib_msg->grids=NULL;
	}
	grib_msg->num_grids=0;
	if ((status = read_func(temp, 4)) != 4) {
		if (status == 0) {
			return -1;
		} else {
			return 1;
		}
	}

	/* search for the beginning of the next GRIB message */
	if (strncmp((char *)temp, "GRIB", 4) != 0) {
		while (temp[0] != 0x47 || temp[1] != 0x52 || temp[2] != 0x49 || temp[3] != 0x42) {
			switch (temp[1]) {
				case 0x47:
					for (n=0; n < 3; n++) {
						temp[n]=temp[n+1];
					}
					if ((status = read_func(&temp[3], 1)) == 0) {
						return -1;
					}
					break;

				default:
					switch(temp[2]) {
						case 0x47:
							for (n = 0; n < 2; n++) {
								temp[n] = temp[n+2];
							}
							if ((status = read_func(&temp[2], 2)) == 0) {
								return -1;
							}
							break;

						default:
							switch(temp[3]) {
								case 0x47:
									temp[0]=temp[3];
									if ((status = read_func(&temp[1], 3)) == 0) {
										return -1;
									}
									break;
								default:
									if ((status = read_func(temp, 4)) == 0) {
										return -1;
									}
									break;
							}
							break;
					}
			}
		}
	}
	if ((status = read_func(&temp[4], 12)) == 0) {
		return 1;
	}
	get_bits(temp, &grib_msg->disc, 48, 8);
	get_bits(temp, &grib_msg->ed_num, 56, 8);
	get_bits(temp, &grib_msg->total_len, 96, 32);
	grib_msg->md.nx = grib_msg->md.ny = 0;
	grib_msg->buffer = (unsigned char *)malloc((grib_msg->total_len + 4) * sizeof(unsigned char));
	memcpy(grib_msg->buffer, temp, 16);
	num = grib_msg->total_len - 16;
	status = read_func(&grib_msg->buffer[16], num);
	if (status != num) {
		return 1;
	} else {
		if (strncmp(&((char *)grib_msg->buffer)[grib_msg->total_len - 4], "7777", 4) != 0) {
			fprintf(stderr, "Warning: no end section found\n");
		}
		grib_msg->offset = 128;
		return 0;
	}
} /* }}} */

void grib2_unpackIDS(GRIBMessage * grib_msg) /* {{{ */
{
	int length;
	int hh;
	int mm;
	int ss;

	/* length of the IDS */
	get_bits(grib_msg->buffer,&length,grib_msg->offset,32);

	/* center ID */
	get_bits(grib_msg->buffer,&grib_msg->center_id,grib_msg->offset+40,16);

	/* sub-center ID */
	get_bits(grib_msg->buffer,&grib_msg->sub_center_id,grib_msg->offset+56,16);

	/* table version */
	get_bits(grib_msg->buffer,&grib_msg->table_ver,grib_msg->offset+72,8);

	/* local table version */
	get_bits(grib_msg->buffer,&grib_msg->local_table_ver,grib_msg->offset+80,8);

	/* significance of reference time */
	get_bits(grib_msg->buffer,&grib_msg->ref_time_type,grib_msg->offset+88,8);

	/* year */
	get_bits(grib_msg->buffer,&grib_msg->yr,grib_msg->offset+96,16);

	/* month */
	get_bits(grib_msg->buffer,&grib_msg->mo,grib_msg->offset+112,8);

	/* day */
	get_bits(grib_msg->buffer,&grib_msg->dy,grib_msg->offset+120,8);

	/* hours */
	get_bits(grib_msg->buffer,&hh,grib_msg->offset+128,8);

	/* minutes */
	get_bits(grib_msg->buffer,&mm,grib_msg->offset+136,8);

	/* seconds */
	get_bits(grib_msg->buffer,&ss,grib_msg->offset+144,8);
	grib_msg->time=hh*10000+mm*100+ss;

	/* production status */
	get_bits(grib_msg->buffer,&grib_msg->prod_status,grib_msg->offset+152,8);

	/* type of data */
	get_bits(grib_msg->buffer,&grib_msg->data_type,grib_msg->offset+160,8);
	grib_msg->offset+=length*8;
} /* }}} */

void grib2_unpackLUS(GRIBMessage * grib)
{
	UNUSED_ARG(grib);
	/* TODO */
}

void grib2_unpackGDS(GRIBMessage * grib_msg) /* {{{ */
{
	int src;
	int num_in_list;
	int sign;
	int value;

	/* source of grid definition */
	get_bits(grib_msg->buffer,&src,grib_msg->offset+40,8);
	if (src != 0) {
		fprintf(stderr,"Don't recognize predetermined grid definitions");
		exit(1);
	}

	/* quasi-regular grid indication */
	get_bits(grib_msg->buffer,&num_in_list,grib_msg->offset+80,8);
	if (num_in_list > 0) {
		fprintf(stderr,"Unable to unpack quasi-regular grids");
		exit(1);
	}

	/* grid definition template number */
	get_bits(grib_msg->buffer,&grib_msg->md.gds_templ_num,grib_msg->offset+96,16);
	switch (grib_msg->md.gds_templ_num) {
		/* Latitude/longitude grid */
		case 0:
		case 40:
			/* shape of the earth */
			get_bits(grib_msg->buffer,&grib_msg->md.earth_shape,grib_msg->offset+112,8);

			/* number of latitudes */
			get_bits(grib_msg->buffer,&grib_msg->md.nx,grib_msg->offset+240,32);

			/* number of longitudes */
			get_bits(grib_msg->buffer,&grib_msg->md.ny,grib_msg->offset+272,32);

			/* latitude of first gridpoint */
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+368,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+369,31);
			grib_msg->md.slat=value/1000000.0;
			if (sign == 1) {
				grib_msg->md.slat=-grib_msg->md.slat;
			}

			/* longitude of first gridpoint */
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+400,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+401,31);
			grib_msg->md.slon=value/1000000.0;
			if (sign == 1) {
				grib_msg->md.slon=-grib_msg->md.slon;
			}

			/* resolution and component flags */
			get_bits(grib_msg->buffer,&grib_msg->md.rescomp,grib_msg->offset+432,8);

			/* latitude of last gridpoint */
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+440,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+441,31);
			grib_msg->md.lats.elat=value/1000000.0;
			if (sign == 1) {
				grib_msg->md.lats.elat=-grib_msg->md.lats.elat;
			}

			/* longitude of last gridpoint */
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+472,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+473,31);
			grib_msg->md.lons.elon=value/1000000.0;
			if (sign == 1) {
				grib_msg->md.lons.elon=-grib_msg->md.lons.elon;
			}

			/* longitude increment */
			get_bits(grib_msg->buffer,&value,grib_msg->offset+504,32);
			grib_msg->md.xinc.loinc=value/1000000.0;

			/* latitude increment */
			get_bits(grib_msg->buffer,&value,grib_msg->offset+536,32);
			if (grib_msg->md.gds_templ_num == 0) {
				grib_msg->md.yinc.lainc=value/1000000.0;
			}

			/* scanning mode flag */
			get_bits(grib_msg->buffer,&grib_msg->md.scan_mode,grib_msg->offset+568,8);
			break;

		case 30: /* Lambert conformal grid */
			get_bits(grib_msg->buffer,&grib_msg->md.earth_shape,grib_msg->offset+112,8);

			/* number of points along a parallel */
			get_bits(grib_msg->buffer,&grib_msg->md.nx,grib_msg->offset+240,32);

			/* number of points along a meridian */
			get_bits(grib_msg->buffer,&grib_msg->md.ny,grib_msg->offset+272,32);

			/* latitude of first gridpoint */
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+304,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+305,31);
			grib_msg->md.slat=value/1000000.0;
			if (sign == 1) {
				grib_msg->md.slat=-grib_msg->md.slat;
			}
			/* longitude of first gridpoint */
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+336,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+337,31);
			grib_msg->md.slon=value/1000000.0;
			if (sign == 1) {
				grib_msg->md.slon=-grib_msg->md.slon;
			}

			/* resolution and component flags */
			get_bits(grib_msg->buffer,&grib_msg->md.rescomp,grib_msg->offset+368,8);

			/* LaD */
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+376,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+377,31);
			grib_msg->md.lats.lad=value/1000000.0;
			if (sign == 1) {
				grib_msg->md.lats.lad=-grib_msg->md.lats.lad;
			}

			/* LoV */
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+408,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+409,31);
			grib_msg->md.lons.lov=value/1000000.0;
			if (sign == 1) {
				grib_msg->md.lons.lov=-grib_msg->md.lons.lov;
			}

			/* x-direction increment */
			get_bits(grib_msg->buffer,&value,grib_msg->offset+440,32);
			grib_msg->md.xinc.dxinc=value/1000.0;

			/* y-direction increment */
			get_bits(grib_msg->buffer,&value,grib_msg->offset+472,32);
			grib_msg->md.yinc.dyinc=value/1000.0;

			/* projection center flag */
			get_bits(grib_msg->buffer,&grib_msg->md.proj_flag,grib_msg->offset+504,8);

			/* scanning mode flag */
			get_bits(grib_msg->buffer,&grib_msg->md.scan_mode,grib_msg->offset+512,8);

			/* latin1 */
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+520,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+521,31);
			grib_msg->md.latin1=value/1000000.0;
			if (sign == 1) {
				grib_msg->md.latin1=-grib_msg->md.latin1;
			}

			/* latin2 */
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+552,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+553,31);
			grib_msg->md.latin2=value/1000000.0;
			if (sign == 1) {
				grib_msg->md.latin2=-grib_msg->md.latin2;
			}

			/* latitude of southern pole of projection */
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+584,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+585,31);
			grib_msg->md.splat=value/1000000.;
			if (sign == 1) {
				grib_msg->md.splat=-grib_msg->md.splat;
			}

			/* longitude of southern pole of projection */
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+616,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+617,31);
			grib_msg->md.splon=value/1000000.;
			if (sign == 1) {
				grib_msg->md.splon=-grib_msg->md.splon;
			}
			break;

		default:
			fprintf(stderr,"Grid template %d is not understood\n",grib_msg->md.gds_templ_num);
			exit(1);
			break;
	}
} /* }}} */

void grib2_unpackPDS(GRIBMessage * grib_msg) /* {{{ */
{
	int num_coords;
	int factor;
	int sign;
	int value;
	int hh;
	int mm;
	int ss;
	int n;
	size_t off;
	size_t start;

	/* indication of hybrid coordinate system */
	get_bits(grib_msg->buffer, &num_coords,grib_msg->offset + 40, 16);
	if (num_coords > 0) {
		fprintf(stderr,"Unable to decode hybrid coordinates");
		exit(1);
	}
	/* product definition template number */
	get_bits(grib_msg->buffer, &grib_msg->md.pds_templ_num, grib_msg->offset + 56, 16);
	grib_msg->md.stat_proc.num_ranges = 0;
	switch (grib_msg->md.pds_templ_num) {
		case 0:
		case 1:
		case 2:
		case 8:
		case 11:
		case 12:
			grib_msg->md.ens_type=-1;
			grib_msg->md.derived_fcst_code=-1;

			/* parameter category */
			get_bits(grib_msg->buffer,&grib_msg->md.param_cat,grib_msg->offset+72,8);

			/* parameter number */
			get_bits(grib_msg->buffer,&grib_msg->md.param_num,grib_msg->offset+80,8);

			/* generating process */
			get_bits(grib_msg->buffer,&grib_msg->md.gen_proc,grib_msg->offset+88,8);

			/* time range indicator*/
			get_bits(grib_msg->buffer,&grib_msg->md.time_unit,grib_msg->offset+136,8);

			/* forecast time */
			get_bits(grib_msg->buffer,&grib_msg->md.fcst_time,grib_msg->offset+144,32);

			/* type of first level */
			get_bits(grib_msg->buffer,&grib_msg->md.lvl1_type,grib_msg->offset+176,8);

			/* value of first level */
			get_bits(grib_msg->buffer,&factor,grib_msg->offset+184,8);
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+192,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+193,31);
			if (sign == 1) {
				value=-value;
			}
			grib_msg->md.lvl1 = (double)value/pow(10.0,(double)factor);

			/* type of second level */
			get_bits(grib_msg->buffer,&grib_msg->md.lvl2_type,grib_msg->offset+224,8);

			/* value of second level */
			get_bits(grib_msg->buffer,&factor,grib_msg->offset+232,8);
			get_bits(grib_msg->buffer,&sign,grib_msg->offset+240,1);
			get_bits(grib_msg->buffer,&value,grib_msg->offset+241,31);
			if (sign == 1) {
				value=-value;
			}
			grib_msg->md.lvl2=(double)value/pow(10.0,(double)factor);
			start = 272;
			switch (grib_msg->md.pds_templ_num) {
				case 1:
					get_bits(grib_msg->buffer,&grib_msg->md.ens_type,grib_msg->offset+272,8);
					get_bits(grib_msg->buffer,&grib_msg->md.perturb_num,grib_msg->offset+280,8);
					get_bits(grib_msg->buffer,&grib_msg->md.nfcst_in_ensemble,grib_msg->offset+288,8);
					break;
				case 2:
					get_bits(grib_msg->buffer,&grib_msg->md.derived_fcst_code,grib_msg->offset+272,8);
					get_bits(grib_msg->buffer,&grib_msg->md.nfcst_in_ensemble,grib_msg->offset+280,8);
					break;
				case 8:
				case 11:
				case 12:
					if (grib_msg->md.pds_templ_num == 11) {
						get_bits(grib_msg->buffer,&grib_msg->md.ens_type,grib_msg->offset+272,8);
						get_bits(grib_msg->buffer,&grib_msg->md.perturb_num,grib_msg->offset+280,8);
						get_bits(grib_msg->buffer,&grib_msg->md.nfcst_in_ensemble,grib_msg->offset+288,8);
						start=296;
					} else if (grib_msg->md.pds_templ_num == 12) {
						get_bits(grib_msg->buffer,&grib_msg->md.derived_fcst_code,grib_msg->offset+272,8);
						get_bits(grib_msg->buffer,&grib_msg->md.nfcst_in_ensemble,grib_msg->offset+280,8);
						start=288;
					}

					/* end year */
					get_bits(grib_msg->buffer,&grib_msg->md.stat_proc.eyr,grib_msg->offset+start,16);

					/* end month */
					get_bits(grib_msg->buffer,&grib_msg->md.stat_proc.emo,grib_msg->offset+start+16,8);

					/* end day */
					get_bits(grib_msg->buffer,&grib_msg->md.stat_proc.edy,grib_msg->offset+start+24,8);

					/* hours */
					get_bits(grib_msg->buffer,&hh,grib_msg->offset+start+32,8);

					/* minutes */
					get_bits(grib_msg->buffer,&mm,grib_msg->offset+start+40,8);

					/* seconds */
					get_bits(grib_msg->buffer,&ss,grib_msg->offset+start+48,8);
					grib_msg->md.stat_proc.etime=hh*10000+mm*100+ss;

					/* number of time range specifications */
					get_bits(grib_msg->buffer,&grib_msg->md.stat_proc.num_ranges,grib_msg->offset+start+56,8);

					/* number of values missing from process */
					get_bits(grib_msg->buffer,&grib_msg->md.stat_proc.nmiss,grib_msg->offset+start+64,32);
					if (grib_msg->md.stat_proc.proc_code != NULL) {
						free(grib_msg->md.stat_proc.proc_code);
						free(grib_msg->md.stat_proc.incr_type);
						free(grib_msg->md.stat_proc.time_unit);
						free(grib_msg->md.stat_proc.time_length);
						free(grib_msg->md.stat_proc.incr_unit);
						free(grib_msg->md.stat_proc.incr_length);
						grib_msg->md.stat_proc.proc_code = NULL;
					}
					grib_msg->md.stat_proc.proc_code=(int *)malloc(grib_msg->md.stat_proc.num_ranges*sizeof(int));
					grib_msg->md.stat_proc.incr_type=(int *)malloc(grib_msg->md.stat_proc.num_ranges*sizeof(int));
					grib_msg->md.stat_proc.time_unit=(int *)malloc(grib_msg->md.stat_proc.num_ranges*sizeof(int));
					grib_msg->md.stat_proc.time_length=(int *)malloc(grib_msg->md.stat_proc.num_ranges*sizeof(int));
					grib_msg->md.stat_proc.incr_unit=(int *)malloc(grib_msg->md.stat_proc.num_ranges*sizeof(int));
					grib_msg->md.stat_proc.incr_length=(int *)malloc(grib_msg->md.stat_proc.num_ranges*sizeof(int));
					off = start + 96;
					for (n = 0; n < grib_msg->md.stat_proc.num_ranges; n++) {
						get_bits(grib_msg->buffer,&grib_msg->md.stat_proc.proc_code[n],grib_msg->offset+off,8);
						get_bits(grib_msg->buffer,&grib_msg->md.stat_proc.incr_type[n],grib_msg->offset+off+8,8);
						get_bits(grib_msg->buffer,&grib_msg->md.stat_proc.time_unit[n],grib_msg->offset+off+16,8);
						get_bits(grib_msg->buffer,&grib_msg->md.stat_proc.time_length[n],grib_msg->offset+off+24,32);
						get_bits(grib_msg->buffer,&grib_msg->md.stat_proc.incr_unit[n],grib_msg->offset+off+56,8);
						get_bits(grib_msg->buffer,&grib_msg->md.stat_proc.incr_length[n],grib_msg->offset+off+64,32);
						off += 96;
					}
					break;
			}
			break;

		default:
			fprintf(stderr,"Product Definition Template %d is not understood\n", grib_msg->md.pds_templ_num);
			exit(1);
			break;
	}
} /* }}} */

void grib2_unpackDRS(GRIBMessage * grib) /* {{{ */
{
	int sign;
	int value;

	/* number of packed values */
	get_bits(grib->buffer,&grib->md.num_packed,grib->offset+40,32);
	/* data representation template number */
	get_bits(grib->buffer,&grib->md.drs_templ_num,grib->offset+72,16);
	switch (grib->md.drs_templ_num) {
		case 0:
		case 40:
		case 40000:
			get_bits(grib->buffer, (int *)&grib->md.R,grib->offset + 88, 32);
			get_bits(grib->buffer, &sign, grib->offset + 120, 1);
			get_bits(grib->buffer, &value, grib->offset + 121, 15);
			if (sign == 1) {
				value = -value;
			}
			grib->md.E = value;
			get_bits(grib->buffer,&sign,grib->offset+136,1);
			get_bits(grib->buffer,&value,grib->offset+137,15);
			if (sign == 1) {
				value = -value;
			}
			grib->md.D = value;
			grib->md.R /= pow(10.0, grib->md.D);
			get_bits(grib->buffer, &grib->md.pack_width, grib->offset + 152, 8);
			break;

		default:
			fprintf(stderr,"Data template %d is not understood\n",grib->md.drs_templ_num);
			exit(1);
			break;
	}
} /* }}} */

void grib2_unpackBMS(GRIBMessage * grib) /* {{{ */
{
	int ind;
	int len;
	int n;
	int bit;

	/* bit map indicator */
	get_bits(grib->buffer, &ind, grib->offset + 40, 8);
	switch (ind) {
		case 0:
			get_bits(grib->buffer, &len, grib->offset, 32);
			len = (len - 6) * 8;
			grib->md.bitmap = (unsigned char *)malloc(len * sizeof(unsigned char));
			for (n = 0; n < len; n++) {
				get_bits(grib->buffer, &bit, grib->offset + 48 + n, 1);
				grib->md.bitmap[n] = bit;
			}
			break;
		case 254:
			break;
		case 255:
			grib->md.bitmap = NULL;
			break;
		default:
			fprintf(stderr,"This code is not currently set up to deal with predefined bit-maps\n");
			exit(1);
			break;
	}
} /* }}} */

void grib2_unpackDS(GRIBMessage * grib, int grid_num) /* {{{ */
{
	int off;
	int n;
	int pval;
	int len;
	int * jvals;
	int cnt;

	off = grib->offset + 40;
	switch (grib->md.drs_templ_num) {
		case 0:
			(grib->grids[grid_num]).gridpoints=(double *)malloc(grib->md.ny * grib->md.nx * sizeof(double));
			for (n=0; n < grib->md.ny * grib->md.nx; n++) {
				if (grib->md.bitmap == NULL || grib->md.bitmap[n] == 1) {
					get_bits(grib->buffer, &pval, off, grib->md.pack_width);
					grib->grids[grid_num].gridpoints[n] = grib->md.R+pval
						* pow(2.0, grib->md.E) / pow(10.0, grib->md.D);
					off += grib->md.pack_width;
				} else {
					grib->grids[grid_num].gridpoints[n] = GRIB_MISSING_VALUE;
				}
			}
			break;
		case 40:
		case 40000:
			get_bits(grib->buffer, &len,grib->offset, 32);
			len = len - 5;
			jvals = (int *)malloc(grib->md.ny * grib->md.nx * sizeof(int));
			(grib->grids[grid_num]).gridpoints = (double *)malloc(grib->md.ny * grib->md.nx * sizeof(double));
			if (len > 0) {
				dec_jpeg2000((char *)&grib->buffer[grib->offset / 8 + 5], len, jvals);
			}
			cnt = 0;
			for (n = 0; n < grib->md.ny * grib->md.nx; n++) {
				if (grib->md.bitmap == NULL || grib->md.bitmap[n] == 1) {
					if (len == 0) {
						jvals[cnt] = 0;
					}
					grib->grids[grid_num].gridpoints[n] = grib->md.R
						+ jvals[cnt++] * pow(2.0, grib->md.E) / pow(10.0, grib->md.D);
				} else {
					grib->grids[grid_num].gridpoints[n] = GRIB_MISSING_VALUE;
				}
			}
			free(jvals);
			break;
	}
} /* }}} */

int grib2_unpack(GRIBMessage * grib, int (*read_func)(void * buf, unsigned int len))
{
	int n;
	int off;
	int len;
	int sec_num;
	int status;

	if (read_func == NULL) {
		return -1;
	}

	status = grib2_unpackIS(grib, read_func);
	if (status != 0) {
		return status;
	}
	grib2_unpackIDS(grib);

	/* find out how many grids are in this message */
	off = grib->offset;
	while (strncmp(&((char *)grib->buffer)[off/8], "7777", 4) != 0) {
		get_bits(grib->buffer, &len,off, 32);
		get_bits(grib->buffer, &sec_num, off + 32, 8);
		if (sec_num == 7) {
			grib->num_grids++;
		}
		off += len * 8;
	}
	grib->grids = (GRIB2Grid *)malloc(grib->num_grids * sizeof(GRIB2Grid));
	n = 0;
	while (strncmp(&((char *)grib->buffer)[grib->offset/8], "7777", 4) != 0) {
		get_bits(grib->buffer, &len, grib->offset, 32);
		get_bits(grib->buffer, &sec_num, grib->offset + 32, 8);
		switch (sec_num) {
			case 2:
				grib2_unpackLUS(grib);
				break;
			case 3:
				grib2_unpackGDS(grib);
				break;
			case 4:
				grib2_unpackPDS(grib);
				break;
			case 5:
				grib2_unpackDRS(grib);
				break;
			case 6:
				grib2_unpackBMS(grib);
				break;
			case 7:
				grib->grids[n].md = grib->md;
				grib2_unpackDS(grib, n);
				n++;
				break;
		}
		grib->offset += len * 8;
	}
	return 0;
}

