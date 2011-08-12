#ifndef __GRIB2__HPP__
#define __GRIB2__HPP__

#include <vector>
#include <istream>
#include <stdint.h>

#if !defined(GRIB_MISSING_VALUE)
#define GRIB_MISSING_VALUE (1.e30)
#endif

namespace grib2 {

/*
struct Meta
{
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
		int num_ranges;
		int nmiss;
		std::vector<int> proc_code;
		std::vector<int> incr_type;
		std::vector<int> time_unit;
		std::vector<int> time_length;
		std::vector<int> incr_unit;
		std::vector<int> incr_length;
	} stat_proc;
	int drs_templ_num;
	float R;
	int E;
	int D;
	int num_packed;
	int pack_width;
	int bms_ind;
	std::vector<unsigned char> bitmap;
};

struct Grid
{
	Meta meta;
	std::vector<double> data;
};
*/

struct indicator_section_t
{
	uint8_t discipline;
	uint8_t edition;
	uint64_t length; // total length of GRIB message in octets (all sections)
};

struct identification_section_t
{
	uint32_t length; // octets
	uint8_t number;
	uint16_t originating_center;
	uint16_t originating_subcenter;
	uint8_t master_table_version;
	uint8_t local_table_version;
	uint8_t significance_ref_time;
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t production_status;
	uint8_t data_type;
};

struct local_use_section_t
{
	uint32_t length;
	uint8_t number;

	// TODO: data for local use?
};

struct grid_definition_section_t
{
	uint32_t length;
	uint8_t number;
	uint8_t source;
	uint32_t num_datapoints;
	uint8_t num_optional;
	uint8_t interpol_list;
	uint16_t grid_def_templ;

	// TODO
};

struct message_t
{
	indicator_section_t is;
	identification_section_t ids;
	local_use_section_t lus; // TODO: may occur multiple times
	grid_definition_section_t gds; // TODO: may occur multiple times
};

class exception
{};

}

std::istream & operator>>(std::istream &, grib2::indicator_section_t &) throw (grib2::exception);
std::istream & operator>>(std::istream &, grib2::identification_section_t &) throw (grib2::exception);
std::istream & operator>>(std::istream &, grib2::local_use_section_t &) throw (grib2::exception);
std::istream & operator>>(std::istream &, grib2::grid_definition_section_t &) throw (grib2::exception);

#endif
