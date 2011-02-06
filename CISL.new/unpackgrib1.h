#ifndef __UNPACKGRIB1__H__
#define __UNPACKGRIB1__H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int total_len;
	int pds_len;
	int pds_ext_len;
	int gds_len;
	int bds_len;
	int ed_num;
	int table_ver;
	int center_id;
	int gen_proc;
	int grid_type;
	int param;
	int level_type;
	int lvl1;
    int lvl2;
	int fcst_units;
	int p1;
	int p2;
	int t_range;
	int navg;
	int nmiss;
	int sub_center_id;
	int bds_flag;
	int pack_width;
	int gds_included;
	int bms_included;
	int yr;
	int mo;
	int dy;
	int time;
	int offset; /* offset in bytes to next GRIB section */
	int D;
	int data_rep;
	int nx;
	int ny;
	int rescomp;
	int scan_mode;
	int proj;
	double slat;
	double slon;
	double elat;
	double elon;
	double lainc;
	double loinc;
	double olon;
	int xlen;
	int ylen;
	unsigned char * buffer;
	unsigned char * pds_ext;
	double ref_val;
	double ** gridpoints;
	int ngy;
} GRIBRecord;

void grib1_unpackGDS(GRIBRecord * grib);
void grib1_unpackBDS(GRIBRecord * grib);
void grib1_unpackPDS(GRIBRecord * grib),
int grib1_unpackIS(FILE * fp, GRIBRecord * grib);
int grib1_unpack(FILE * fp, GRIBRecord * grib);

#ifdef __cplusplus
}
#endif

#endif
