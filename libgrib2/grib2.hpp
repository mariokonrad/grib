#ifndef __GRIB2__HPP__
#define __GRIB2__HPP__

#include <vector>
#include <istream>
#include <stdint.h>

#include <string>
#include <sstream>

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
	uint64_t total_length; // total length of GRIB message in octets (all sections)
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

	std::vector<char> data;
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

	union grid_def_t
	{
		struct lat_lon_t // template 3.0
		{
			uint8_t shape_earth; // table 3.2
			uint8_t scale_factor_radius;
			uint32_t scale_value_radius;
			uint8_t scale_factor_major_axis;
			uint32_t scale_value_major_axis;
			uint8_t scale_factor_minor_axis;
			uint32_t scale_value_minor_axis;
			uint32_t num_parallel; // number of points along parallel
			uint32_t num_meridian; // number of points along meridian
			uint32_t basic_angle;
			uint32_t subdiv_basic_angle;
			uint32_t lat1;
			uint32_t lon1;
			uint8_t resolution;
			uint32_t lat2;
			uint32_t lon2;
			uint32_t di; // i-direction increment
			uint32_t dj; // j-direction increment
			uint8_t scanning_mode;
			// TODO: list of number points along each meridian or parallel
		} lat_lon;
		// TODO
	} grid_def;
};

struct product_definition_section_t
{
	uint32_t length;
	uint8_t number;
	uint16_t num_coord_values;
	uint16_t product_def_templ;

	union prod_def_t
	{
		struct info_t
		{
			uint8_t param_category;
			uint8_t param_number;
			uint8_t type_gen_proc;
			uint8_t bg_gen_proc;
			uint8_t gen_proc_id;
			uint16_t hours_obs_data_cutoff;
			uint8_t minutes_obs_data_cutoff;
			uint8_t indicator_unit_of_timerange;
			uint32_t forecast_time;
			uint8_t type_first_fix_surf;
			uint8_t scale_factor_first_fix_surf;
			uint32_t scale_value_first_fix_surf;
			uint8_t type_second_fix_surf;
			uint8_t scale_factor_second_fix_surf;
			uint32_t scale_value_second_fix_surf;
		} info;
		// TODO
	} prod_def;

	// TODO
};

struct data_representation_section_t
{
	uint32_t length;
	uint8_t number;
	uint32_t num_datapoints;
	uint16_t rep_templ;

	union rep_def_t {
		struct gp_simple_t // template 5.0
		{
			union {
				uint32_t u;
				float f;
			} R; // IEEE-754 binary 32bit float: reference value
			int16_t E; // binary scale factor
			int16_t D; // decimal scale factor
			uint8_t num_bits;
			uint8_t type_org; // code table 5.1
		} gp_simple;
		struct gp_jpeg2000_t // templte 5.40
		{
			union {
				uint32_t u;
				float f;
			} R; // IEEE-754 binary 32bit float: reference value
			int16_t E; // binary scale factor
			int16_t D; // decimal scale factor
			uint8_t num_bits;
			uint8_t type_org; // code table 5.1
			uint8_t type_compression; // code table 5.40
			uint8_t target_compression_ratio;
		} gp_jpeg2000;
		struct gp_png_t // template 5.41
		{
			union {
				uint32_t u;
				float f;
			} R; // IEEE-754 binary 32bit float: reference value
			int16_t E; // binary scale factor
			int16_t D; // decimal scale factor
			uint8_t num_bits;
			uint8_t type_org; // code table 5.1
		} gp_png;
		struct sd_simple_t // template 5.50
		{
			union {
				uint32_t u;
				float f;
			} R; // IEEE-754 binary 32bit float: reference value
			int16_t E; // binary scale factor
			int16_t D; // decimal scale factor
			uint8_t num_bits;
			uint32_t real;
		} sd_simple;
	} rep_def;
};

struct bitmap_section_t
{
	uint32_t length;
	uint8_t number;
	uint8_t bitmap_indicator;
	std::vector<char> bitmap;
};

struct data_section_t
{
	uint32_t length;
	uint8_t number;
	std::vector<double> data;
};

struct message_t
{
	indicator_section_t is;
	identification_section_t ids;
	local_use_section_t lus;
	grid_definition_section_t gds;
	product_definition_section_t pds;
	data_representation_section_t drs;
	bitmap_section_t bm;
	data_section_t ds;
};

class not_implemented : public std::exception
{
	private:
		const char * file;
		int line;
		std::string text;
	private:
		void update()
		{
			std::ostringstream os;
			os << file << ":" << line;
			text = os.str();
		}
	public:
		not_implemented()
			: file("unknown")
			, line(0)
		{
			update();
		}

		not_implemented(const char * file, int line)
			: file(file)
			, line(line)
		{
			update();
		}

		virtual ~not_implemented() throw ()
		{}

		virtual const char * what() const throw ()
		{
			return text.c_str();
		}
};

int unpack(message_t &, std::istream &);

}

#endif
