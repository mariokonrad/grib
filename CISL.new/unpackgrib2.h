#ifndef __UNPACKGRIB2__H__
#define __UNPACKGRIB2__H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int gds_templ_num;
	int earth_shape;
	int nx;
	int ny;
	double slat;
	double slon;
	double latin1;
	double latin2;
	double splat;
	double splon;
	union {
		double elat;
		double lad;
	} lats;
	union {
		double elon;
		double lov;
	} lons;
	union {
		double loinc;
		double dxinc;
	} xinc;
	union {
		double lainc;
		double dyinc;
	} yinc;
	int rescomp;
	int scan_mode;
	int proj_flag;
	int pds_templ_num;
	int param_cat;
	int param_num;
	int gen_proc;
	int time_unit;
	int fcst_time;
	int ens_type;
	int perturb_num;
	int derived_fcst_code;
	int nfcst_in_ensemble;
	int lvl1_type;
	int lvl2_type;
	double lvl1;
	double lvl2;
	struct {
		int eyr;
		int emo;
		int edy;
		int etime;
		int num_ranges,nmiss;
		int * proc_code;
		int * incr_type;
		int * time_unit;
		int * time_length;
		int * incr_unit;
		int * incr_length;
	} stat_proc;
	int drs_templ_num;
	float R;
	int E;
	int D;
	int num_packed;
	int pack_width;
	int bms_ind;
	unsigned char * bitmap;
} GRIBMetadata;

typedef struct {
	GRIBMetadata md;
	double *gridpoints;
} GRIB2Grid;

typedef struct {
	unsigned char * buffer;
	int offset;  /* offset in bytes to next GRIB2 section */
	int total_len;
	int disc,ed_num;
	int center_id;
	int sub_center_id;
	int table_ver;
	int local_table_ver;
	int ref_time_type;
	int yr;
	int mo;
	int dy;
	int time;
	int prod_status;
	int data_type;
	GRIBMetadata md;
	int num_grids;
	GRIB2Grid * grids;
} GRIBMessage;

int grib2_unpackIS(FILE * fp,GRIBMessage * grib_msg);
void grib2_unpackIDS(GRIBMessage * grib_msg);
void grib2_unpackLUS(GRIBMessage * grib);
void grib2_unpackGDS(GRIBMessage * grib_msg);
void grib2_unpackPDS(GRIBMessage * grib_msg);
void grib2_unpackDRS(GRIBMessage * grib_msg);
void grib2_unpackBMS(GRIBMessage * grib_msg);
void grib2_unpackDS(GRIBMessage * grib_msg, int grid_num);
int grib2_unpack(FILE * fp,GRIBMessage * grib);

#ifdef __cplusplus
}
#endif

#endif
