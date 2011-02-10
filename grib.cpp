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
#include <curl/curl.h>
#include <grib2_conv.h>

class GRIB // {{{
{
	private:
		typedef std::vector<std::string> Levels;
		typedef std::vector<std::string> Vars;
	public:
		enum Grid { GRID_2P5, GRID_1P0, GRID_0P5 };
		enum Cycle { CYCLE_00, CYCLE_06, CYCLE_12, CYCLE_18 };

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
		typedef Data::const_iterator const_iterator;
		typedef Data::value_type value_type;
	private:
		Levels levels;
		Vars vars;
		Data data;
		Grid grid;
		Cycle cycle;
		Region region;
		int time;
	private:
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

		void add_level(const std::string &);
		void add_var(const std::string &);

		static std::string error_string(int);
		static bool check_time(Grid, int);

		std::string url() const;
		int fetch();

		friend std::ostream & operator<<(std::ostream &, const GRIB &);

		const_iterator begin() const;
		const_iterator end() const;
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

void GRIB::get_levels(std::ostream & os) const
{
	for (Levels::const_iterator i = levels.begin(); i != levels.end(); ++i)
		os << "&lev_" << *i << "=on";
}

void GRIB::get_vars(std::ostream & os) const
{
	for (Vars::const_iterator i = vars.begin(); i != vars.end(); ++i)
		os << "&var_" << *i << "=on";
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
	printf("%p / %u / %u / %u\n", buf, size, n, data.size()); // TODO:TEMP
	return N;
}

void GRIB::add_level(const std::string & s)
{
	levels.push_back(s);
}

void GRIB::add_var(const std::string & s)
{
	vars.push_back(s);
}

size_t GRIB::write_callback(char * buf, size_t size, size_t n, void * userptr)
{
	GRIB * grib = reinterpret_cast<GRIB *>(userptr);
	if (!grib) return 0;
	return grib->write(buf, size, n);
}

int GRIB::fetch()
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
	curl_easy_setopt(curl, CURLOPT_URL, url().c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GRIB::write_callback);
	CURLcode res = curl_easy_perform(curl);
	int rc = 0;
	switch (res) {
		case CURLE_OK:
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

GRIB::const_iterator GRIB::begin() const
{
	return data.begin();
}

GRIB::const_iterator GRIB::end() const
{
	return data.end();
}

// }}}

static GRIB::const_iterator grib2_begin;
static GRIB::const_iterator grib2_i;
static GRIB::const_iterator grib2_end;
static GRIB::Data grib1;

static int read_func(void * buf, unsigned int len)
{
	if (grib2_i == grib2_end) return 0;

}

static int write_func(const void * buf, unsigned int len)
{
}

int main(int, char **)
{
	for (int i = 0; i < 6; i += 3) {
		GRIB grib(GRIB::GRID_2P5);
		grib.set(GRIB::Region(35, 5, -15, 15));
		grib.set(GRIB::CYCLE_00);
		grib.set_time(i);
		grib.add_level("mean_sea_level");
		grib.add_level("surface");
		grib.add_var("GUST");
		grib.add_var("HGT");
		grib.add_var("PRES");
		grib.add_var("PRMSL");
		grib.add_var("TMP");

		printf("url: [%s]\n", grib.url().c_str());
		if (grib.fetch()) {
			printf("ERROR: %s\n", GRIB::error_string(errno).c_str());
			break;
		}

		grib2_begin = grib.begin();
		grib2_i = grib.begin();
		grib2_end = grib.end();
		grib2_to_grib1_conv(read_func, write_func);

		std::ofstream ofs("test.grb2");
		if (!ofs) {
			printf("ERROR: cannot open file\n");
			break;
		}
		std::copy(grib.begin(), grib.end(), std::ostream_iterator<GRIB::value_type>(ofs));
	}

	return 0;
}

