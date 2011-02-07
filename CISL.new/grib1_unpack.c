#include <grib1_unpack.h>
#include <get_bits.h>
#include <conv_float.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int grib1_unpackIS(FILE * fp, GRIBRecord * grib) /* {{{ */
{
	unsigned char temp[8];
	int status;
	size_t n;
	size_t num;

	if (grib->buffer != NULL) {
		free(grib->buffer);
	}
	if ((status=fread(temp, 1, 4, fp)) != 4) {
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
					for (n = 0; n < 3; n++) {
						temp[n] = temp[n+1];
					}
					if ((status = fread(&temp[3], 1, 1, fp)) == 0) {
						return -1;
					}
					break;
				default:
					switch (temp[2]) {
						case 0x47:
							for (n = 0; n < 2; n++) {
								temp[n] = temp[n+2];
							}
							if ((status = fread(&temp[2], 1, 2, fp)) == 0) {
								return -1;
							}
							break;
						default:
							switch(temp[3]) {
								case 0x47:
									temp[0] = temp[3];
									if ((status = fread(&temp[1], 1, 3, fp)) == 0) {
										return -1;
									}
									break;
								default:
									if ((status = fread(temp, 1, 4, fp)) == 0) {
										return -1;
									}
									break;
							}
					}
					break;
			}
		}
	}

	if ((status = fread(&temp[4], 1, 4, fp)) == 0) {
		return 1;
	}
	get_bits(temp, &grib->total_len, 32, 24);
	if (grib->total_len == 24) {
		grib->ed_num = 0;
		grib->pds_len = grib->total_len;

		/* add the four bytes for 'GRIB' + 3 bytes for the length of the section
		 ** following the PDS */
		grib->total_len += 7;
	} else {
		grib->ed_num = 1;
	}

	grib->nx = grib->ny = 0;
	grib->buffer = (unsigned char *)malloc(grib->total_len + 4);
	memcpy(grib->buffer, temp, 8);
	num = grib->total_len - 8;
	status = fread(&grib->buffer[8], 1, num, fp);
	if (status != num) {
		return 1;
	} else {
		if (strncmp(&((char *)grib->buffer)[grib->total_len-4], "7777", 4) != 0) {
			fprintf(stderr,"Warning: no end section found\n");
		}
		return 0;
	}
} /* }}} */

void grib1_unpackPDS(GRIBRecord * grib) /* {{{ */
{
	int n;
	int flag;
	int min;
	int cent;
	int sign;
	char * c_buf = (char *)grib->buffer;

	if (grib->ed_num == 0) {
		grib->offset = 32;
	} else {
		grib->offset = 64;

		/* length of PDS */
		get_bits(grib->buffer, &grib->pds_len, grib->offset, 24);

		/* table version */
		get_bits(grib->buffer, &grib->table_ver, grib->offset + 24, 8);
	}

	/* center ID */
	get_bits(grib->buffer, &grib->center_id, grib->offset + 32, 8);

	/* generating process */
	get_bits(grib->buffer, &grib->gen_proc, grib->offset + 40, 8);

	/* grid type */
	get_bits(grib->buffer, &grib->grid_type, grib->offset + 48, 8);
	get_bits(grib->buffer, &flag, grib->offset + 56, 8);

	/* indication of GDS */
	grib->gds_included = ((flag & 0x80) == 0x80) ? 1 : 0;

	/* indication of BMS */
	grib->bms_included = ((flag & 0x40) == 0x40) ? 1 : 0;

	/* parameter code */
	get_bits(grib->buffer, &grib->param, grib->offset + 64, 8);

	/* level type */
	get_bits(grib->buffer, &grib->level_type, grib->offset + 72, 8);
	switch (grib->level_type) {
		case 100:
		case 103:
		case 105:
		case 107:
		case 109:
		case 111:
		case 113:
		case 115:
		case 125:
		case 160:
		case 200:
		case 201:
			/* first level */
			get_bits(grib->buffer, &grib->lvl1, grib->offset + 80, 16);
			grib->lvl2 = 0;
			break;

		default:
			/* first level */
			get_bits(grib->buffer, &grib->lvl1, grib->offset + 80, 8);

			/* second level */
			get_bits(grib->buffer, &grib->lvl2, grib->offset + 88, 8);
			break;
	}

	/* year of the century */
	get_bits(grib->buffer ,&grib->yr, grib->offset + 96, 8);

	/* month */
	get_bits(grib->buffer, &grib->mo, grib->offset + 104, 8);

	/* day */
	get_bits(grib->buffer, &grib->dy, grib->offset + 112, 8);

	/* hour */
	get_bits(grib->buffer, &grib->time, grib->offset + 120, 8);

	/* minutes */
	get_bits(grib->buffer, &min, grib->offset + 128, 8);

	/* complete time */
	grib->time = grib->time * 100 + min;

	/* forecast time units */
	get_bits(grib->buffer, &grib->fcst_units, grib->offset + 136, 8);

	/* P1 */
	get_bits(grib->buffer, &grib->p1, grib->offset + 144, 8);

	/* P2 */
	get_bits(grib->buffer, &grib->p2, grib->offset + 152, 8);

	/* time range indicator*/
	get_bits(grib->buffer, &grib->t_range, grib->offset + 160, 8);
	switch (grib->p2) {
		case 3:
		case 4:
		case 51:
		case 113:
		case 114:
		case 115:
		case 116:
		case 117:
		case 123:
		case 124:
			/* number in average */
			get_bits(grib->buffer, &grib->navg, grib->offset + 168, 16);
			break;

		default:
			/* number in average */
			grib->navg = 0;
			break;
	}
	/* missing grids in average */
	get_bits(grib->buffer, &grib->nmiss, grib->offset + 184, 8);

	/* if GRIB0, done with PDS */
	if (grib->ed_num == 0) {
		grib->pds_ext_len = 0;
		grib->offset += 192;
		return;
	}

	get_bits(grib->buffer, &cent, grib->offset + 192, 8);  /* century */
	grib->yr += (cent - 1) * 100;

	/* sub-center ID */
	get_bits(grib->buffer, &grib->sub_center_id, grib->offset + 200, 8);
	get_bits(grib->buffer, &sign, grib->offset + 208, 1);

	/* decimal scale factor */
	get_bits(grib->buffer, &grib->D, grib->offset + 209, 15);
	if (sign == 1) {
		grib->D =- grib->D;
	}

	grib->offset += 224;
	if (grib->pds_len > 28) {
		if (grib->pds_ext != NULL) {
			free(grib->pds_ext);
			grib->pds_ext = NULL;
		}
		if (grib->pds_len < 40) {
			fprintf(stderr,"Warning: PDS extension is in wrong location\n");
			grib->pds_ext_len = grib->pds_len - 28;
			grib->pds_ext = (unsigned char *)malloc(grib->pds_ext_len);
			for (n = 0; n < grib->pds_ext_len; n++) {
				grib->pds_ext[n] = c_buf[36 + n];
			}
			grib->offset += grib->pds_ext_len * 8;
		} else {
			grib->pds_ext_len = grib->pds_len - 40;
			grib->pds_ext = (unsigned char *)malloc(grib->pds_ext_len);
			for (n = 0; n < grib->pds_ext_len; n++) {
				grib->pds_ext[n] = c_buf[48 + n];
			}
			grib->offset += (grib->pds_ext_len + 12) * 8;
		}
	} else {
		grib->pds_ext_len = 0;
	}
} /* }}} */

void grib1_unpackGDS(GRIBRecord * grib) /* {{{ */
{
	int sign;
	int dum;

	/* length of the GDS */
	get_bits(grib->buffer, &grib->gds_len, grib->offset, 24);

	if (grib->ed_num == 0) {
		grib->total_len += grib->gds_len;
	}

	/* data representation type */
	get_bits(grib->buffer, &grib->data_rep, grib->offset + 40, 8);
	switch (grib->data_rep) {
		case 0: /* Latitude/Longitude grid */
		case 4: /* Gaussian Lat/Lon grid */
		case 10: /* Rotated Lat/Lon grid */
			/* number of latitudes */
			get_bits(grib->buffer, &grib->nx, grib->offset + 48, 16);

			/* number of longitudes */
			get_bits(grib->buffer, &grib->ny, grib->offset + 64, 16);
			get_bits(grib->buffer, &sign, grib->offset + 80, 1);
			get_bits(grib->buffer, &dum, grib->offset + 81, 23);

			/* latitude of first gridpoint */
			grib->slat = dum * 0.001;
			if (sign == 1) {
				grib->slat =- grib->slat;
			}
			get_bits(grib->buffer, &sign, grib->offset + 104, 1);
			get_bits(grib->buffer, &dum, grib->offset + 105, 23);

			/* longitude of first gridpoint */
			grib->slon = dum * 0.001;
			if (sign == 1) {
				grib->slon =- grib->slon;
			}

			/* resolution and component flags */
			get_bits(grib->buffer, &grib->rescomp, grib->offset + 128, 8);
			get_bits(grib->buffer, &sign, grib->offset + 136, 1);
			get_bits(grib->buffer, &dum, grib->offset + 137, 23);

			/* latitude of last gridpoint */
			grib->elat = dum * 0.001;
			if (sign == 1) {
				grib->elat =- grib->elat;
			}
			get_bits(grib->buffer, &sign, grib->offset + 160, 1);
			get_bits(grib->buffer, &dum, grib->offset + 161, 23);

			/* longitude of last gridpoint */
			grib->elon = dum * 0.001;
			if (sign == 1) {
				grib->elon =- grib->elon;
			}
			get_bits(grib->buffer, &dum, grib->offset + 184, 16);

			/* longitude increment */
			grib->loinc = dum * 0.001;
			get_bits(grib->buffer, &dum, grib->offset + 200, 16);

			/* latitude increment */
			if (grib->data_rep == 0) {
				grib->lainc = dum * 0.001;
			} else {
				grib->lainc = dum;
			}

			/* scanning mode flag */
			get_bits(grib->buffer,&grib->scan_mode,grib->offset+216,8);
			break;

		case 3: /* Lambert Conformal grid */
		case 5: /* Polar Stereographic grid */
			/* number of x-points */
			get_bits(grib->buffer,&grib->nx,grib->offset+48,16);

			/* number of y-points */
			get_bits(grib->buffer,&grib->ny,grib->offset+64,16);
			get_bits(grib->buffer,&sign,grib->offset+80,1);
			get_bits(grib->buffer,&dum,grib->offset+81,23);

			/* latitude of first gridpoint */
			grib->slat = dum * 0.001;
			if (sign == 1) {
				grib->slat =- grib->slat;
			}
			get_bits(grib->buffer, &sign, grib->offset + 104, 1);
			get_bits(grib->buffer, &dum, grib->offset + 105, 23);

			/* longitude of first gridpoint */
			grib->slon = dum * 0.001;
			if (sign == 1) {
				grib->slon =- grib->slon;
			}

			/* resolution and component flags */
			get_bits(grib->buffer, &grib->rescomp, grib->offset + 128, 8);
			get_bits(grib->buffer, &sign, grib->offset + 136, 1);
			get_bits(grib->buffer, &dum, grib->offset + 137, 23);

			/* longitude of grid orientation */
			grib->olon = dum * 0.001;
			if (sign == 1) {
				grib->olon =- grib->olon;
			}

			/* x-direction grid length */
			get_bits(grib->buffer, &grib->xlen, grib->offset + 160, 24);

			/* y-direction grid length */
			get_bits(grib->buffer, &grib->ylen, grib->offset + 184, 24);

			/* projection center flag */
			get_bits(grib->buffer, &grib->proj, grib->offset + 208, 8);

			/* scanning mode flag */
			get_bits(grib->buffer, &grib->scan_mode, grib->offset + 216, 8);
			break;

		default:
			fprintf(stderr,"Grid type %d is not understood\n", grib->data_rep);
			exit(1);
			break;
	}
	grib->offset += grib->gds_len * 8;
} /* }}} */

void grib1_unpackBDS(GRIBRecord * grib) /* {{{ */
{
	int n;
	int m;
	int boff;
	int bcnt = 0;
	int num_packed = 0;
	int bms_length;
	int sign;
	int ub;
	int tref;
	int * bitmap = NULL;
	int E;
	int * packed = NULL;
	int cnt;
	double e;
	double d = pow(10.0, grib->D);

	if (grib->bms_included == 1) {
		get_bits(grib->buffer, &bms_length, grib->offset, 24);

		if (grib->ed_num == 0) {
			grib->total_len += bms_length;
		}
		get_bits(grib->buffer, &ub, grib->offset + 24, 8);
		get_bits(grib->buffer, &tref, grib->offset + 32, 16);
		if (tref != 0) {
			fprintf(stderr,"Error: unknown pre-defined bit-map %d\n",tref);
			exit(1);
		}
		num_packed = (bms_length - 6) * 8 - ub;
		bitmap = (int *)malloc(sizeof(int)*num_packed);
		boff = grib->offset + 48;
		for (n = 0; n < num_packed; n++) {
			get_bits(grib->buffer, &bitmap[n], boff, 1);
			boff++;
		}
		grib->offset += bms_length * 8;
	}

	/* length of the BDS */
	get_bits(grib->buffer,&grib->bds_len,grib->offset,24);

	if (grib->ed_num == 0) {
		grib->total_len += (grib->bds_len + 1);
	}
	/* flag */
	get_bits(grib->buffer, &grib->bds_flag, grib->offset + 24, 4);
	get_bits(grib->buffer, &ub,grib->offset + 28, 4);
	/* bit width of the packed data points */
	get_bits(grib->buffer, &grib->pack_width, grib->offset + 80, 8);

	get_bits(grib->buffer, &sign, grib->offset + 32, 1);
	/* binary scale factor */
	get_bits(grib->buffer, &E, grib->offset + 33, 15);
	if (sign == 1) {
		E =- E;
	}
	e = pow(2.0, E);

	/* reference value */
	grib->ref_val = ibm2real(grib->buffer, grib->offset + 48) / d;

	if ((grib->bds_flag & 0x40) == 0) {
		/* simple packing */
		grib->offset += 88;
		if (grib->pack_width > 0) {
			num_packed = (grib->bds_len * 8 - 88 - ub) / grib->pack_width;
			packed = (int *)malloc(sizeof(int)*num_packed);
		}
		cnt = 0;
		switch (grib->data_rep) {
			case 0: /* Latitude/Longitude grid */
			case 4: /* Gaussian Lat/Lon grid */
			case 10: /* Rotated Lat/Lon grid */
				switch (grib->grid_type) {
					case 23:
					case 24:
					case 26:
					case 63:
					case 64:
						grib->offset += grib->pack_width;
						break;
				}
			case 3: /* Lambert Conformal grid */
			case 5: /* Polar Stereographic grid */
				for (n = 0; n < num_packed; n++) {
					get_bits(grib->buffer, &packed[n], grib->offset, grib->pack_width);
					grib->offset += grib->pack_width;
				}

				if (grib->gridpoints != NULL) {
					for (n = 0; n < grib->ngy; n++) {
						free(grib->gridpoints[n]);
					}
					free(grib->gridpoints);
				}

				grib->ngy = grib->ny;
				grib->gridpoints = (double **)malloc(sizeof(double) * grib->ngy);
				for (n = 0; n < grib->ngy; n++) {
					grib->gridpoints[n] = (double *)malloc(sizeof(double) * grib->nx);
				}
				for (n = 0; n < grib->ny; n++) {
					for (m = 0; m < grib->nx; m++) {
						if (bitmap == NULL || (bitmap != NULL && bitmap[bcnt++] == 1)) {
							if (packed == NULL) {
								/* constant field */
								grib->gridpoints[n][m] = grib->ref_val;
							} else {
								grib->gridpoints[n][m] = grib->ref_val + packed[cnt++] * e / d;
							}
						} else {
							grib->gridpoints[n][m] = GRIB_MISSING_VALUE;
						}
					}
				}
				break;
				/* no recognized GDS, so just unpack the stream of gridpoints */
			default:
				grib->ngy = grib->ny = 1;
				grib->gridpoints = (double **)malloc(sizeof(double)*grib->ngy);
				for (n = 0; n < grib->ngy; n++) {
					grib->gridpoints[n] = (double *)malloc(sizeof(double)*grib->nx);
				}
				for (n = 0; n < num_packed; n++) {
					if (bitmap == NULL || (bitmap != NULL && bitmap[bcnt++] == 1)) {
						grib->gridpoints[0][n] = grib->ref_val+packed[n] * e / d;
					} else {
						grib->gridpoints[0][n] = GRIB_MISSING_VALUE;
					}
					grib->nx++;
				}
				break;
		}
	} else {
		/* second-order packing */
		fprintf(stderr,"Error: complex packing not currently supported\n");
		exit(1);
	}
	free(packed);
} /* }}} */

int grib1_unpack(FILE * fp, GRIBRecord * grib)
{
	int status;

	status = grib1_unpackIS(fp, grib);
	if (status != 0) {
		return status;
	}
	grib1_unpackPDS(grib);
	if (grib->gds_included == 1) {
		grib1_unpackGDS(grib);
	}
	grib1_unpackBDS(grib);

	return 0;
}

