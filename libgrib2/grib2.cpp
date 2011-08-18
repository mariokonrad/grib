#include <grib2.hpp>
#include <iostream>
#include <cmath>
#include <bitset.hpp>

// http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc.shtml

#define EPSILON (1.0e-9)

namespace grib2 {

typedef bitset<uint8_t> octets;

static void unpack(std::istream &, indicator_section_t &) throw (std::exception);
static void unpack(std::istream &, identification_section_t &) throw (std::exception);
static void unpack(std::istream &, local_use_section_t &) throw (std::exception);
static void unpack(std::istream &, grid_definition_section_t &) throw (std::exception);
static void unpack(std::istream &, product_definition_section_t &) throw (std::exception);
static void unpack(std::istream &, data_representation_section_t &) throw (std::exception);
static void unpack(std::istream &, bitmap_section_t &) throw (std::exception);
static void unpack(std::istream &, data_section_t &, const data_representation_section_t &) throw (std::exception);

template <typename T> static void read(std::istream & is, T & value) throw (std::exception)
{
	octets buf;
	buf.reserve(sizeof(T));
	if (buf.append(is, sizeof(T)) != sizeof(T)) throw std::exception();
	octets::const_iterator i = buf.begin();
	i.read(value);
}

static int search_next_message(std::istream & is) // {{{
{
	int state = 0;
	int8_t c;

	while (is.good() && !is.eof()) {
		is >> c;
		switch (state) {
			case 0:
				state = (c == 'G') ? 1 : 0;
				break;
			case 1:
				state = (c == 'R') ? 2 : 0;
				break;
			case 2:
				state = (c == 'I') ? 3 : 0;
				break;
			case 3:
				if (c == 'B') return 0;
				state = 0;
				break;
		}
	}
	return -1;
} // }}}

int unpack(message_t & grib, std::istream & is)
{
	uint32_t section_length;
	uint8_t section_number;
	uint64_t total_length = 0;

	if (search_next_message(is) < 0) return -1; // consumes "GRIB" indicator
	try {
		unpack(is, grib.is);
		total_length += 16;
		total_length += 4; // "7777" at the end, FIXME: read from "GRIB" to "7777" into vector and read data from there
		while (total_length < grib.is.total_length) {
			read(is, section_length);
			if (section_length == 0x37373737) break; // "7777" = end of grib message
			read(is, section_number);

			switch (section_number) {
				case 1:
					grib.ids.length = section_length;
					grib.ids.number = section_number;
					unpack(is, grib.ids);
					break;

				case 2:
					grib.lus.length = section_length;
					grib.lus.number = section_number;
					unpack(is, grib.lus);
					break;

				case 3:
					grib.gds.length = section_length;
					grib.gds.number = section_number;
					unpack(is, grib.gds);
					break;

				case 4:
					grib.pds.length = section_length;
					grib.pds.number = section_number;
					unpack(is, grib.pds);
					break;

				case 5:
					grib.drs.length = section_length;
					grib.drs.number = section_number;
					unpack(is, grib.drs);
					break;

				case 6:
					grib.bm.length = section_length;
					grib.bm.number = section_number;
					unpack(is, grib.bm);
					break;

				case 7:
					grib.ds.length = section_length;
					grib.ds.number = section_number;
					unpack(is, grib.ds, grib.drs);
					break;

				default:
					std::cout
						<< "UNKNOWN SECTION: "
						<< static_cast<int>(section_number)
						<< " (length="
						<< section_length
						<< "). exit."
						<< std::endl;
					return -1;
			}
			total_length += section_length;
		}
	} catch (octets::exception &) {
		std::cerr << "OCTET READ EXCEPTION" << std::endl;
		return -2;
	} catch (grib2::not_implemented e) {
		std::cerr << "NOT IMPLEMENTED: " << e.what() << std::endl;
		return -1;
	} catch (std::exception & e) {
		std::cerr << "EXCEPTION: " << e.what() << std::endl;
		return -1;
	} catch (...) {
		std::cerr << "UNKNOWN EXCEPTION" << std::endl;
		return -1;
	}
	return 0;
}

static void unpack(std::istream & is, grib2::indicator_section_t & section) throw (std::exception)
{
	uint16_t reserved;
	grib2::octets buf;

	buf.reserve(12); // ignoring "GRIB" indicator
	if (buf.append(is, 12) != 12) throw std::exception();
	grib2::octets::const_iterator i = buf.begin();
	i.read(reserved);
	i.read(section.discipline);
	i.read(section.edition);
	i.read(section.total_length);
}

static void unpack(std::istream & is, grib2::identification_section_t & section) throw (std::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw std::exception();
	grib2::octets::const_iterator i = buf.begin();

	i.read(section.originating_center);
	i.read(section.originating_subcenter);
	i.read(section.master_table_version);
	i.read(section.local_table_version);
	i.read(section.significance_ref_time);
	i.read(section.year);
	i.read(section.month);
	i.read(section.day);
	i.read(section.hour);
	i.read(section.minute);
	i.read(section.second);
	i.read(section.production_status);
	i.read(section.data_type);

	// all additional data is reserved
}

static void unpack(std::istream & is, grib2::local_use_section_t & section) throw (std::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw std::exception();
	section.data.clear();
	grib2::octets::const_iterator i = buf.begin();

	// copy local data
	for (; i != buf.end(); ++i) {
		section.data.push_back(*i);
	}
}

static void unpack_GDS_3_0(grib2::octets::const_iterator & i, grib2::grid_definition_section_t & section) throw (std::exception)
{
	struct grib2::grid_definition_section_t::grid_def_t::lat_lon_t & t = section.grid_def.lat_lon;
	struct grib2::grid_definition_section_t::grid_def_t::calc_t & calc = section.grid_def.calc;

	i.read(t.shape_earth);
	i.read(t.scale_factor_radius);
	i.read(t.scale_value_radius);
	i.read(t.scale_factor_major_axis);
	i.read(t.scale_value_major_axis);
	i.read(t.scale_factor_minor_axis);
	i.read(t.scale_value_minor_axis);
	i.read(t.num_parallel);
	i.read(t.num_meridian);
	i.read(t.basic_angle);
	i.read(t.subdiv_basic_angle);
	i.read(t.lat1);
	i.read(t.lon1);
	i.read(t.resolution);
	i.read(t.lat2);
	i.read(t.lon2);
	i.read(t.di);
	i.read(t.dj);
	i.read(t.scanning_mode);

	/*
	TODO : 73-nn: List of number of points along each meridian or parallel
	(These octets are only present for quasi-regular grids as described in notes 2 and 3)
	*/

	calc.lat1 = static_cast<double>(t.lat1) * 1.0e-6;
	calc.lon1 = static_cast<double>(t.lon1) * 1.0e-6;
	calc.lat2 = static_cast<double>(t.lat2) * 1.0e-6;
	calc.lon2 = static_cast<double>(t.lon2) * 1.0e-6;
}

static void unpack(std::istream & is, grib2::grid_definition_section_t & section) throw (std::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw std::exception();
	grib2::octets::const_iterator i = buf.begin();

	i.read(section.source);
	i.read(section.num_datapoints);
	i.read(section.num_optional);
	i.read(section.interpol_list);
	i.read(section.grid_def_templ);

	switch (section.grid_def_templ) { // table 3.1
		case 0: // Latitude/Longitude (template 3.0)
			unpack_GDS_3_0(i, section);
			break;
		case 1: // Rotated Latitude/Longitude (template 3.1)
		case 2: // Streched Latitude/Longitude (template 3.2)
		case 3: // Rotated and Streched Latitude/Longitude (template 3.3)
		case 10: // Mecrator (template 3.10)
		case 20: // Polar Stereographic Projection (North or South) (See Template 3.20)
		case 30: // Lambert Conformal (Secant, tangent, conical, or bipolar) (See Template 3.30)
		case 31: // Albers Equal Area (See Template 3.31)
		case 40: // Gaussian Latitude/Longitude (See Template 3.40)
		case 41: // Rotated Gaussian Latitude/Longitude (See Template 3.41)
		case 42: // Stretched Gaussian Latitude/Longitude (See Template 3.42)
		case 43: // Rotated and Stretched Gaussian Latitude/Longitude (See Template 3.43)
		case 50: // Spherical Harmonic Coefficients (See Template 3.50)
		case 51: // Rotated Spherical Harmonic Coefficients (See Template 3.51)
		case 52: // Stretched Spherical Harmonic Coefficients (See Template 3.52)
		case 53: // Rotated and Stretched Spherical Harmonic Coefficients (See Template 3.53)
		case 90: // Space View Perspective or Orthographic (See Template 3.90)
		case 100: // Triangular Grid Based on an Icosahedron (See Template 3.100)
		case 110: // Equatorial Azimuthal Equidistant Projection (See Template 3.110)
		case 120: // Azimuth-Range Projection (See Template 3.120)
		case 204: // Curvilinear Orthogonal Grids (See Template 3.204)
		case 1000: // Cross Section Grid with Points Equally Spaced on the Horizontal (See Template 3.1000)
		case 1100: // Hovmoller Diagram with Points Equally Spaced on the Horizontal (See Template 3.1100)
		case 1200: // Time Section Grid (See Template 3.1200)
		case 32768: // Rotated Latitude/Longitude (Arakawa Staggered E-Grid)(See Template 3.32768)
		case 32769: // Rotated Latitude/Longitude (Arakawa Non-E Staggered Grid)(See Template 3.32769)
		case 65535: // Missing
			throw not_implemented(__FILE__, __LINE__);
			break;
		default:
			throw std::exception();
	}

	// TODO: optional list of numbers defineing number of points
}

static void unpack_PDS_4_0(grib2::octets::const_iterator & i, grib2::product_definition_section_t & section) throw (std::exception)
{
	struct grib2::product_definition_section_t::prod_def_t::info_t & t = section.prod_def.info;

	i.read(t.param_category);
	i.read(t.param_number);
	i.read(t.type_gen_proc);
	i.read(t.bg_gen_proc);
	i.read(t.gen_proc_id);
	i.read(t.hours_obs_data_cutoff);
	i.read(t.minutes_obs_data_cutoff);
	i.read(t.indicator_unit_of_timerange);
	i.read(t.forecast_time);
	i.read(t.type_first_fix_surf);
	i.read(t.scale_factor_first_fix_surf);
	i.read(t.scale_value_first_fix_surf);
	i.read(t.type_second_fix_surf);
	i.read(t.scale_factor_second_fix_surf);
	i.read(t.scale_value_second_fix_surf);
}

static void unpack(std::istream & is, grib2::product_definition_section_t & section) throw (std::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw std::exception();
	grib2::octets::const_iterator i = buf.begin();

	i.read(section.num_coord_values);
	i.read(section.product_def_templ);

	switch (section.product_def_templ) { // TODO: table 4.0
		case 0: // Analysis or forecast at a horizontal level or in a horizontal layer at a point in time.  (see Template 4.0)
			unpack_PDS_4_0(i, section);
			break;
		case 1:
			// TODO: many more
			throw not_implemented(__FILE__, __LINE__);
		default:
			throw std::exception();
	}

	// TODO: optional list of coordinate values
}

static void unpack(std::istream & is, grib2::data_representation_section_t & section) throw (std::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw std::exception();
	grib2::octets::const_iterator i = buf.begin();

	i.read(section.num_datapoints);
	i.read(section.rep_templ);

	switch (section.rep_templ) { // table 5.0
		case 0: // Grid Point Data - Simple Packing (see Template 5.0)
			i.read(section.rep_def.gp_simple.R.u); // data already in IEEE-754 binary float 32
			i.read(section.rep_def.gp_simple.E);
			i.read(section.rep_def.gp_simple.D);
			i.read(section.rep_def.gp_simple.num_bits);
			i.read(section.rep_def.gp_simple.type_org);
			break;
		case 1: // Matrix Value at Grid Point - Simple Packing (see Template 5.1)
		case 2: // Grid Point Data - Complex Packing (see Template 5.2)
		case 3: // Grid Point Data - Complex Packing and Spatial Differencing (see Template 5.3)
		case 4: // Grid Point Data - IEEE Floating Point Data (see Template 5.4)
			throw not_implemented(__FILE__, __LINE__);
			break;
		case 40: // Grid Point Data - JPEG2000 Compression (see Template 5.40)
			i.read(section.rep_def.gp_jpeg2000.R.u); // data already in IEEE-754 binary float 32
			i.read(section.rep_def.gp_jpeg2000.E);
			i.read(section.rep_def.gp_jpeg2000.D);
			i.read(section.rep_def.gp_jpeg2000.num_bits);
			i.read(section.rep_def.gp_jpeg2000.type_org);
			break;
		case 41: // Grid Point Data - PNG Compression (see Template 5.41)
			i.read(section.rep_def.gp_png.R.u); // data already in IEEE-754 binary float 32
			i.read(section.rep_def.gp_png.E);
			i.read(section.rep_def.gp_png.D);
			i.read(section.rep_def.gp_png.num_bits);
			i.read(section.rep_def.gp_png.type_org);
			break;
		case 50: // Spectral Data - Simple Packing (see Template 5.50)
			i.read(section.rep_def.sd_simple.R.u); // data already in IEEE-754 binary float 32
			i.read(section.rep_def.sd_simple.E);
			i.read(section.rep_def.sd_simple.D);
			i.read(section.rep_def.sd_simple.num_bits);
			i.read(section.rep_def.sd_simple.real);
			break;
		case 51: // Spectral Data - Complex Packing (see Template 5.51)
		case 61: // Grid Point Data - Simple Packing With Logarithm Pre-processing
		case 200: // Run Length Packing With Level Values (see Template 5.200)
			throw not_implemented(__FILE__, __LINE__);
			break;
		default:
			throw std::exception();
	}
}

static void unpack(std::istream & is, grib2::bitmap_section_t & section) throw (std::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw std::exception();

	grib2::octets::const_iterator i = buf.begin();

	i.read(section.bitmap_indicator); // see table 6.0

	// copy bitmap
	for (; i != buf.end(); ++i) {
		section.bitmap.push_back(*i);
	}
}

static void unpack_DS_5_0(grib2::octets::const_iterator & i, grib2::data_section_t & section,
	const data_representation_section_t & drs) throw (std::exception)
{
	// according to: http://www.wmo.int/pages/prog/www/WDM/Guides/Guide-binary-2.html
	// y * 10^D = R + (x * 2^E) ==> y = (R + (x * 2^E)) * 10^(-D)

	// y = pow(10.0, -D) * (R + x * pow(2.0, E));

	const grib2::data_representation_section_t::rep_def_t::gp_simple_t & def = drs.rep_def.gp_simple;

	uint32_t t;
	double val;

	if (def.num_bits > sizeof(t) * grib2::octets::BITS_PER_BYTE) throw std::exception();

	section.data.clear();
	section.data.reserve(drs.num_datapoints);

	double decimal_scale = pow(10.0, -def.D);
	double binary_scale = pow(2.0, def.E);


	for (uint32_t dp = 0; dp < drs.num_datapoints; ++dp) {
		i.read(t, def.num_bits);
		if (def.num_bits == 0) {
			val = def.R.f;
		} else {
			val = decimal_scale * (def.R.f + t * binary_scale);
		}
std::cerr << __FILE__ << ":" << __LINE__ << ": " << val << "  D=" << def.D << " E=" << def.E << "  R=" << def.R.f << "  t=" << t << std::endl;
		section.data.push_back(val);
	}
std::cerr << std::endl;
}

static void unpack(std::istream & is, data_section_t & section, const data_representation_section_t & drs) throw (std::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw std::exception();
	grib2::octets::const_iterator i = buf.begin();

	switch (drs.rep_templ) { // table 5.0
		case 0: // Grid Point Data - Simple Packing (see Template 5.0)
			unpack_DS_5_0(i, section, drs);
			break;
		case 1: // Matrix Value at Grid Point - Simple Packing (see Template 5.1)
		case 2: // Grid Point Data - Complex Packing (see Template 5.2)
		case 3: // Grid Point Data - Complex Packing and Spatial Differencing (see Template 5.3)
		case 4: // Grid Point Data - IEEE Floating Point Data (see Template 5.4)
		case 40: // Grid Point Data - JPEG2000 Compression (see Template 5.40)
		case 41: // Grid Point Data - PNG Compression (see Template 5.41)
		case 50: // Spectral Data - Simple Packing (see Template 5.50)
		case 51: // Spectral Data - Complex Packing (see Template 5.51)
		case 61: // Grid Point Data - Simple Packing With Logarithm Pre-processing
		case 200: // Run Length Packing With Level Values (see Template 5.200)
			std::cerr << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": value=" << drs.rep_templ << std::endl;
			throw not_implemented(__FILE__, __LINE__);
			break;
		default:
			throw std::exception();
	}
}

}

