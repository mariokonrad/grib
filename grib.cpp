#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <iterator>
#include <vector>
#include <cstdio>
#include <cerrno>
#include <ctime>
#include <stdint.h>
#include <curl/curl.h>

class GRIB // {{{
{
	public:
		enum Grid { GRID_2P5, GRID_1P0, GRID_0P5 };
		enum Cycle { CYCLE_00, CYCLE_06, CYCLE_12, CYCLE_18 };

/* TODO
lev_0-0.1_m_below_ground  0-0.1 m below ground  
lev_0.1-0.4_m_below_ground  0.1-0.4 m below ground  
lev_0.33-1_sigma_layer  0.33-1 sigma layer  
lev_0.4-1_m_below_ground  0.4-1 m below ground  
lev_0.44-0.72_sigma_layer  0.44-0.72 sigma layer  
lev_0.44-1_sigma_layer  0.44-1 sigma layer  
lev_0.72-0.94_sigma_layer  0.72-0.94 sigma layer  
lev_0.995_sigma_level  0.995 sigma level   
lev_0C_isotherm  0C isotherm   
lev_1-2_m_below_ground  1-2 m below ground   
lev_30-0_mb_above_ground  30-0 mb above ground  
lev_180-0_mb_above_ground  180-0 mb above ground  
lev_boundary_layer_cloud_layer  boundary layer cloud layer  
lev_convective_cloud_bottom_level  convective cloud bottom level  
lev_convective_cloud_layer  convective cloud layer  
lev_convective_cloud_top_level  convective cloud top level  
lev_entire_atmosphere_\(considered_as_a_single_layer\)  entire atmosphere (considered as a single layer)  
lev_high_cloud_bottom_level  high cloud bottom level  
lev_high_cloud_layer  high cloud layer  
lev_high_cloud_top_level  high cloud top level  
lev_highest_tropospheric_freezing_level  highest tropospheric freezing level  
lev_low_cloud_bottom_level  low cloud bottom level  
lev_low_cloud_layer  low cloud layer   
lev_low_cloud_top_level  low cloud top level  
lev_middle_cloud_bottom_level  middle cloud bottom level  
lev_middle_cloud_layer  middle cloud layer  
lev_middle_cloud_top_level  middle cloud top level  
lev_PV=\-2e\-06_\(Km\^2/kg/s\)_surface  PV=-2e-06 (Km^2/kg/s) surface  
lev_top_of_atmosphere  top of atmosphere  
*/
		enum Level {
			 LEVEL__ALL
			,LEVEL_MEAN_SEA_LEVEL
			,LEVEL_SURFACE
			,LEVEL_MAX_WIND
			,LEVEL_TROPOPAUSE
			,LEVEL_10_MB
			,LEVEL_20_MB
			,LEVEL_30_MB
			,LEVEL_50_MB
			,LEVEL_70_MB
			,LEVEL_100_MB
			,LEVEL_150_MB
			,LEVEL_200_MB
			,LEVEL_250_MB
			,LEVEL_300_MB
			,LEVEL_350_MB
			,LEVEL_400_MB
			,LEVEL_450_MB
			,LEVEL_500_MB
			,LEVEL_550_MB
			,LEVEL_600_MB
			,LEVEL_650_MB
			,LEVEL_700_MB
			,LEVEL_750_MB
			,LEVEL_800_MB
			,LEVEL_850_MB
			,LEVEL_900_MB
			,LEVEL_925_MB
			,LEVEL_950_MB
			,LEVEL_975_MB
			,LEVEL_1000_MB
			,LEVEL_1829_M_ABOVE_MEAN_SEA_LEVEL
			,LEVEL_2743_M_ABOVE_MEAN_SEA_LEVEL
			,LEVEL_3658_M_ABOVE_MEAN_SEA_LEVEL
			,LEVEL_2_M_ABOVE_GROUND
			,LEVEL_10_M_ABOVE_GROUND
		};

		enum Variable {
			 VAR__ALL
			,VAR_4LFTX
			,VAR_5WAVA
			,VAR_5WAVH
			,VAR_ABSV
			,VAR_ACPCP
			,VAR_ALBDO
			,VAR_APCP
			,VAR_CAPE
			,VAR_CFRZR
			,VAR_CICEP
			,VAR_CIN
			,VAR_CLWMR
			,VAR_CPRAT
			,VAR_CRAIN
			,VAR_CSNOW
			,VAR_CWAT
			,VAR_CWORK
			,VAR_DLWRF
			,VAR_DSWRF
			,VAR_FLDCP
			,VAR_GFLUX
			,VAR_GPA
			,VAR_GUST
			,VAR_HGT
			,VAR_HLCY
			,VAR_HPBL
			,VAR_ICAHT
			,VAR_ICEC
			,VAR_LAND
			,VAR_LFTX
			,VAR_LHTFL
			,VAR_O3MR
			,VAR_PEVPR
			,VAR_POT
			,VAR_PRATE
			,VAR_PRES
			,VAR_PRMSL
			,VAR_PWAT
			,VAR_RH
			,VAR_SHTFL
			,VAR_SOILW
			,VAR_SPFH
			,VAR_SUNSD
			,VAR_TCDC
			,VAR_TMAX
			,VAR_TMIN
			,VAR_TMP
			,VAR_TOZNE
			,VAR_UFLX
			,VAR_UGRD
			,VAR_U_GWD
			,VAR_ULWRF
			,VAR_USTM
			,VAR_USWRF
			,VAR_VFLX
			,VAR_VGRD
			,VAR_V_GWD
			,VAR_VVEL
			,VAR_VWSH
			,VAR_WATR
			,VAR_WEASD
			,VAR_WILT
		};

		class Region
		{
				friend class GRIB;
			private:
				int top_lat;
				int bottom_lat;
				int left_lon;
				int right_lon;
			public:
				Region()
					: top_lat(0)
					, bottom_lat(0)
					, left_lon(0)
					, right_lon(0)
				{}

				Region(int top_lat, int bottom_lat, int left_lon, int right_lon)
					: top_lat(top_lat)
					, bottom_lat(bottom_lat)
					, left_lon(left_lon)
					, right_lon(right_lon)
				{
					if (top_lat > 90) top_lat = 90;
					if (top_lat < -90) top_lat = -90;
					if (bottom_lat > 90) bottom_lat = 90;
					if (bottom_lat < -90) bottom_lat = -90;
					if (left_lon > 180) left_lon = 180;
					if (left_lon < -180) left_lon = -180;
					if (right_lon > 180) right_lon = 180;
					if (right_lon < -180) right_lon = -180;

					if (top_lat < bottom_lat) std::swap(top_lat, bottom_lat);
					if (right_lon < right_lon) std::swap(left_lon, right_lon);
				}

				bool valid() const
				{
					return true
						&& top_lat != bottom_lat
						&& left_lon != right_lon
						;
				}
		};

		typedef std::vector<uint8_t> Data;
		typedef std::pair<GRIB::Data::const_iterator, GRIB::Data::const_iterator> DataRange;
	private:
		typedef std::vector<Level> Levels;
		typedef std::vector<Variable> Vars;
	private:
		Levels levels;
		Vars vars;
		Data data;
		Grid grid;
		Cycle cycle;
		Region region;
		int time;
	private:
		bool has_level_all() const;
		bool has_var_all() const;
		std::string map_level(Level) const;
		std::string map_variable(Variable) const;
		bool has_valid_config() const;
		void ensure_correct_region();
		std::string cycle_str() const;
		std::string time_str() const;
		std::string date_str() const;
		int time_mod() const;
		int time_max() const;
		void get_server(std::ostream &) const;
		void get_script(std::ostream &) const;
		void get_file(std::ostream &) const;
		void get_levels(std::ostream &) const;
		void get_vars(std::ostream &) const;
		void get_region(std::ostream &) const;
		void get_dir(std::ostream &) const;
		size_t write(char *, size_t, size_t);
		static size_t write_callback(char *, size_t, size_t, void *);
	public:
		GRIB(Grid = GRID_2P5);

		void set(Cycle);
		void set(const Region &);
		void set_time(int);
		bool check_time(int) const;

		void add_level(Level);
		void add_var(Variable);

		static std::string error_string(int);
		static bool check_time(Grid, int);

		std::string url() const;
		int fetch(Data &);

		friend std::ostream & operator<<(std::ostream &, const GRIB &);
};

GRIB::GRIB(Grid grid)
	: grid(grid)
	, cycle(CYCLE_00)
	, time(0)
{}

void GRIB::set(Cycle cycle)
{
	this->cycle = cycle;
}

void GRIB::set(const Region & region)
{
	this->region = region;
}

int GRIB::time_mod() const
{
	switch (grid) {
		case GRID_2P5: return 6;
		case GRID_1P0: return 3;
		case GRID_0P5: return 3;
	}
	return 1;
}

int GRIB::time_max() const
{
	switch (grid) {
		case GRID_2P5: return 384;
		case GRID_1P0: return 192;
		case GRID_0P5: return 192;
	}
	return 0;
}

void GRIB::set_time(int time)
{
	if (time < 0) time = 0;
	if (time > time_max()) time = time_max();
	if (time % time_mod() != 0) time -= (time % time_mod());
	this->time = time;
}

bool GRIB::check_time(Grid grid, int time)
{
	return GRIB(grid).check_time(time);
}

bool GRIB::check_time(int time) const
{
	return true
		&& (time % time_mod() == 0)
		&& (time >= 0)
		&& (time <= time_max())
		;
}

bool GRIB::has_valid_config() const
{
	return true
		&& region.valid()
		&& check_time(time)
		;
}

void GRIB::get_server(std::ostream & os) const
{
	os << "http://nomads.ncep.noaa.gov/cgi-bin/";
}

void GRIB::get_script(std::ostream & os) const
{
	switch (grid) {
		case GRID_2P5: os << "filter_gfs_2p5.pl?"; break;
		case GRID_1P0: os << "filter_gfs.pl?"; break;
		case GRID_0P5: os << "filter_gfs_hd.pl?"; break;
	}
}

std::string GRIB::cycle_str() const
{
	switch (cycle) {
		case CYCLE_00: return "00";
		case CYCLE_06: return "06";
		case CYCLE_12: return "12";
		case CYCLE_18: return "18";
	}
	return "";
}

std::string GRIB::time_str() const
{
	std::ostringstream os;
	os << std::setfill('0') << std::setw(2) << time;
	return os.str();
}

std::string GRIB::date_str() const
{
	time_t t;
	::time(&t);

	struct tm utc;
	::gmtime_r(&t, &utc);

	std::ostringstream os;
	os << std::setfill('0') << std::setw(4) << 1900 + utc.tm_year;
	os << std::setfill('0') << std::setw(2) <<    1 + utc.tm_mon;
	os << std::setfill('0') << std::setw(2) <<        utc.tm_mday;
	return os.str();
}

void GRIB::get_file(std::ostream & os) const
{
	os << "file=";
	switch (grid) {
		case GRID_2P5: os << "gfs.t" << cycle_str() << "z.pgrbf" << time_str() << ".2p5deg.grib2"; break;
		case GRID_1P0: os << "gfs.t" << cycle_str() << "z.pgrbf" << time_str() << ".grib2"; break;
		case GRID_0P5: os << "gfs.t" << cycle_str() << "z.mastergrb2f" << time_str(); break;
	}
}

bool GRIB::has_level_all() const
{
	for (Levels::const_iterator i = levels.begin(); i != levels.end(); ++i) {
		if (*i == LEVEL__ALL) return true;
	}
	return false;
}

bool GRIB::has_var_all() const
{
	for (Vars::const_iterator i = vars.begin(); i != vars.end(); ++i) {
		if (*i == VAR__ALL) return true;
	}
	return false;
}

void GRIB::get_levels(std::ostream & os) const
{
	if (has_level_all()) {
		os << "&all_lev=on";
		return;
	}
	for (Levels::const_iterator i = levels.begin(); i != levels.end(); ++i) {
		std::string s = map_level(*i);
		if (s.size()) os << "&lev_" << s << "=on";
	}
}

void GRIB::get_vars(std::ostream & os) const
{
	if (has_var_all()) {
		os << "&all_var=on";
		return;
	}
	for (Vars::const_iterator i = vars.begin(); i != vars.end(); ++i) {
		std::string s = map_variable(*i);
		if (s.size()) os << "&var_" << s << "=on";
	}
}

void GRIB::get_region(std::ostream & os) const
{
	os << "&subregion=";
	os << "&leftlon="   << region.left_lon;
	os << "&rightlon="  << region.right_lon;
	os << "&toplat="    << region.top_lat;
	os << "&bottomlat=" << region.bottom_lat;
}

void GRIB::get_dir(std::ostream & os) const
{
	os << "&dir=%2F";
	switch (grid) {
		case GRID_2P5: os << "gfs." << date_str() << cycle_str(); break;
		case GRID_1P0: os << "gfs." << date_str() << cycle_str(); break;
		case GRID_0P5: os << "gfs." << date_str() << cycle_str() << "%2Fmaster"; break;
	}
}

std::string GRIB::url() const
{
	std::ostringstream os;
	get_server(os);
	get_script(os);
	get_file(os);
	get_levels(os);
	get_vars(os);
	get_region(os);
	get_dir(os);
	return os.str();
}

size_t GRIB::write(char * buf, size_t size, size_t n)
{
	const size_t N = size * n;
	data.reserve(data.size() + N);
	for (size_t i = 0; i < N; ++i) data.push_back(buf[i]);
	return N;
}

std::string GRIB::map_level(Level level) const
{
	switch (level) {
		case LEVEL_MEAN_SEA_LEVEL: return "mean_sea_level";
		case LEVEL_SURFACE:        return "surface";
		case LEVEL_MAX_WIND:       return "max_wind";
		case LEVEL_TROPOPAUSE:     return "tropopause";
		case LEVEL_10_MB:          return "10_mb";
		case LEVEL_20_MB:          return "20_mb";
		case LEVEL_30_MB:          return "30_mb";
		case LEVEL_50_MB:          return "50_mb";
		case LEVEL_70_MB:          return "70_mb";
		case LEVEL_100_MB:         return "100_mb";
		case LEVEL_150_MB:         return "150_mb";
		case LEVEL_200_MB:         return "200_mb";
		case LEVEL_250_MB:         return "250_mb";
		case LEVEL_300_MB:         return "300_mb";
		case LEVEL_350_MB:         return "350_mb";
		case LEVEL_400_MB:         return "400_mb";
		case LEVEL_450_MB:         return "450_mb";
		case LEVEL_500_MB:         return "500_mb";
		case LEVEL_550_MB:         return "550_mb";
		case LEVEL_600_MB:         return "600_mb";
		case LEVEL_650_MB:         return "650_mb";
		case LEVEL_700_MB:         return "700_mb";
		case LEVEL_750_MB:         return "750_mb";
		case LEVEL_800_MB:         return "800_mb";
		case LEVEL_850_MB:         return "850_mb";
		case LEVEL_900_MB:         return "900_mb";
		case LEVEL_925_MB:         return "925_mb";
		case LEVEL_950_MB:         return "950_mb";
		case LEVEL_975_MB:         return "975_mb";
		case LEVEL_1000_MB:        return "1000_mb";
		case LEVEL_1829_M_ABOVE_MEAN_SEA_LEVEL: return "1829_m_above_mean_sea_level";
		case LEVEL_2743_M_ABOVE_MEAN_SEA_LEVEL: return "2743_m_above_mean_sea_level";
		case LEVEL_3658_M_ABOVE_MEAN_SEA_LEVEL: return "3658_m_above_mean_sea_level";
		case LEVEL_2_M_ABOVE_GROUND:  return "2_m_above_ground";
		case LEVEL_10_M_ABOVE_GROUND: return "10_m_above_ground";

		case LEVEL__ALL: break;
	}
	return "";
}

std::string GRIB::map_variable(Variable var) const
{
	switch (var) {
		case VAR_4LFTX: return "4LFTX";
		case VAR_5WAVA: return "5WAVA";
		case VAR_5WAVH: return "5WAVH";
		case VAR_ABSV:  return "ABSV";
		case VAR_ACPCP: return "ACPCP";
		case VAR_ALBDO: return "ALBDO";
		case VAR_APCP:  return "APCP";
		case VAR_CAPE:  return "CAPE";
		case VAR_CFRZR: return "CFRZR";
		case VAR_CICEP: return "CICEP";
		case VAR_CIN:   return "CIN";
		case VAR_CLWMR: return "CLWMR";
		case VAR_CPRAT: return "CPRAT";
		case VAR_CRAIN: return "CRAIN";
		case VAR_CSNOW: return "CSNOW";
		case VAR_CWAT:  return "CWAT";
		case VAR_CWORK: return "CWORK";
		case VAR_DLWRF: return "DLWRF";
		case VAR_DSWRF: return "DSWRF";
		case VAR_FLDCP: return "FLDCP";
		case VAR_GFLUX: return "GFLUX";
		case VAR_GPA:   return "GPA";
		case VAR_GUST:  return "GUST";
		case VAR_HGT:   return "HGT";
		case VAR_HLCY:  return "HLCY";
		case VAR_HPBL:  return "HPBL";
		case VAR_ICAHT: return "ICAHT";
		case VAR_ICEC:  return "ICEC";
		case VAR_LAND:  return "LAND";
		case VAR_LFTX:  return "LFTX";
		case VAR_LHTFL: return "LHTFL";
		case VAR_O3MR:  return "O3MR";
		case VAR_PEVPR: return "PEVPR";
		case VAR_POT:   return "POT";
		case VAR_PRATE: return "PRATE";
		case VAR_PRES:  return "PRES";
		case VAR_PRMSL: return "PRMSL";
		case VAR_PWAT:  return "PWAT";
		case VAR_RH:    return "RH";
		case VAR_SHTFL: return "SHTFL";
		case VAR_SOILW: return "SOILW";
		case VAR_SPFH:  return "SPFH";
		case VAR_SUNSD: return "SUNSD";
		case VAR_TCDC:  return "TCDC";
		case VAR_TMAX:  return "TMAX";
		case VAR_TMIN:  return "TMIN";
		case VAR_TMP:   return "TMP";
		case VAR_TOZNE: return "TOZNE";
		case VAR_UFLX:  return "UFLX";
		case VAR_UGRD:  return "UGRD";
		case VAR_U_GWD: return "U_GWD";
		case VAR_ULWRF: return "ULWRF";
		case VAR_USTM:  return "USTM";
		case VAR_USWRF: return "USWRF";
		case VAR_VFLX:  return "VFLX";
		case VAR_VGRD:  return "VGRD";
		case VAR_V_GWD: return "V_GWD";
		case VAR_VVEL:  return "VVEL";
		case VAR_VWSH:  return "VWSH";
		case VAR_WATR:  return "WATR";
		case VAR_WEASD: return "WEASD";
		case VAR_WILT:  return "WILT";

		case VAR__ALL: break;
	}
	return "";
}

void GRIB::add_level(Level level)
{
	levels.push_back(level);
}

void GRIB::add_var(Variable var)
{
	vars.push_back(var);
}

size_t GRIB::write_callback(char * buf, size_t size, size_t n, void * userptr)
{
	GRIB * grib = reinterpret_cast<GRIB *>(userptr);
	if (!grib) return 0;
	return grib->write(buf, size, n);
}

int GRIB::fetch(Data & result)
{
	if (!has_valid_config()) {
		errno = 0;
		return -1;
	}

	CURL * curl = curl_easy_init();
	if (!curl) {
		errno = EFAULT;
		return -1;
	}
	data.clear();
	curl_easy_setopt(curl, CURLOPT_URL, url().c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GRIB::write_callback);
	CURLcode res = curl_easy_perform(curl);
	int rc = 0;
	switch (res) {
		case CURLE_OK:
			result = data;
			rc = 0;
			break;
		default:
			rc = -1;
			errno = res;
			break;
	}
	curl_easy_cleanup(curl);
	return rc;
}

std::string GRIB::error_string(int code)
{
	return curl_easy_strerror(static_cast<CURLcode>(code));
}

std::ostream & operator<<(std::ostream & os, const GRIB & grib)
{
	return os << grib.url();
}

// }}}

/* DISABLED {{{
static int read_func(void * buf, unsigned int len, void * ptr)
{
	if (ptr == NULL) return 0;
	GRIB::DataRange * range = static_cast<GRIB::DataRange *>(ptr);
	int rc = 0;
	GRIB::Data::value_type * p = static_cast<GRIB::Data::value_type *>(buf);
	for (unsigned int i = 0; i < len && range->first < range->second; ++i, ++rc, ++p, ++(range->first)) {
		*p = *(range->first);
	}
	return rc;
}

static int write_func(const void * buf, unsigned int len, void * ptr)
{
	if (ptr == NULL) return 0;
	GRIB::Data * grib1 = static_cast<GRIB::Data *>(ptr);
	for (unsigned int i = 0; i < len; ++i) {
		grib1->push_back(*(static_cast<const GRIB::Data::value_type *>(buf)+i));
	}
	return len;
}
}}} */

int main(int, char **)
{
	GRIB grib(GRIB::GRID_1P0);
	grib.set(GRIB::Region(35, 5, -15, 15));
	grib.set(GRIB::CYCLE_00);
	grib.add_var(GRIB::VAR_UGRD);
	grib.add_var(GRIB::VAR_VGRD);
	grib.add_level(GRIB::LEVEL_500_MB);
	grib.set_time(0);

	GRIB::Data grib2;
	printf("url: [%s]\n", grib.url().c_str());
	if (grib.fetch(grib2)) {
		printf("ERROR: %s\n", GRIB::error_string(errno).c_str());
		return -1;
	}

	if (grib2.size()) {
		std::ofstream ofs("test.grb2");
		if (ofs) {
			std::copy(grib2.begin(), grib2.end(), std::ostream_iterator<GRIB::Data::value_type>(ofs));
		} else {
			printf("ERROR: cannot open file\n");
		}
	}

	return 0;
}

