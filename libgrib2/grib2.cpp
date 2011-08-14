#include <grib2.hpp>
#include <octets.hpp>
#include <iostream>

// http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc.shtml

namespace grib2 {

static void unpack(std::istream &, indicator_section_t &) throw (grib2::exception);
static void unpack(std::istream &, identification_section_t &) throw (grib2::exception);
static void unpack(std::istream &, local_use_section_t &) throw (grib2::exception);
static void unpack(std::istream &, grid_definition_section_t &) throw (grib2::exception);
static void unpack(std::istream &, product_definition_section_t &) throw (grib2::exception);
static void unpack(std::istream &, data_representation_section_t &) throw (grib2::exception);
static void unpack(std::istream &, bitmap_section_t &) throw (grib2::exception);
static void unpack(std::istream &, data_section_t &, const data_representation_section_t &) throw (grib2::exception);

template <typename T> static void read(std::istream & is, T & value) throw (grib2::exception)
{
	octets buf;
	buf.reserve(sizeof(T));
	if (buf.append(is, sizeof(T)) != sizeof(T)) throw grib2::exception();
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
	} catch (grib2::exception &) {
		std::cerr << "GRIB2 EXCEPTION" << std::endl;
		return -1;
	}
	return 0;
}

static void unpack(std::istream & is, grib2::indicator_section_t & section) throw (grib2::exception)
{
	uint16_t reserved;
	grib2::octets buf;

	buf.reserve(12); // ignoring "GRIB" indicator
	if (buf.append(is, 12) != 12) throw grib2::exception();
	grib2::octets::const_iterator i = buf.begin();
	i.read(reserved);
	i.read(section.discipline);
	i.read(section.edition);
	i.read(section.total_length);
}

static void unpack(std::istream & is, grib2::identification_section_t & section) throw (grib2::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw grib2::exception();
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

static void unpack(std::istream & is, grib2::local_use_section_t & section) throw (grib2::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw grib2::exception();
	section.data.clear();
	grib2::octets::const_iterator i = buf.begin();

	// copy local data
	for (; i != buf.end(); ++i) {
		section.data.push_back(*i);
	}
}

static void unpack(std::istream & is, grib2::grid_definition_section_t & section) throw (grib2::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw grib2::exception();
	grib2::octets::const_iterator i = buf.begin();

	i.read(section.source);
	i.read(section.num_datapoints);
	i.read(section.num_optional);
	i.read(section.interpol_list);
	i.read(section.grid_def_templ);

	switch (section.grid_def_templ) { // TODO: table 3.1
		case 0: // Latitude/Longitude (template 3.0)
			// TODO
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
			throw not_implemented();
			break;
		default: throw exception();
	}

	// TODO: optional list of numbers defineing number of points
}

static void unpack(std::istream & is, grib2::product_definition_section_t & section) throw (grib2::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw grib2::exception();
	grib2::octets::const_iterator i = buf.begin();

	i.read(section.num_coord_values);
	i.read(section.product_def_templ);

	switch (section.product_def_templ) { // TODO: table 4.0
		case 0: // Analysis or forecast at a horizontal level or in a horizontal layer at a point in time.  (see Template 4.0)
		// TODO: many more
			throw not_implemented();
			break;
		default: throw exception();
	}

	// TODO: optional list of coordinate values
}

static void unpack(std::istream & is, grib2::data_representation_section_t & section) throw (grib2::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw grib2::exception();
	grib2::octets::const_iterator i = buf.begin();

	i.read(section.num_datapoints);
	i.read(section.rep_templ);

	switch (section.rep_templ) { // table 5.0
		case 0: // Grid Point Data - Simple Packing (see Template 5.0)
			i.read(section.templ.gp_simple.R);
			i.read(section.templ.gp_simple.E);
			i.read(section.templ.gp_simple.D);
			i.read(section.templ.gp_simple.num_bits);
			i.read(section.templ.gp_simple.type_org);
			break;
		case 1: // Matrix Value at Grid Point - Simple Packing (see Template 5.1)
		case 2: // Grid Point Data - Complex Packing (see Template 5.2)
		case 3: // Grid Point Data - Complex Packing and Spatial Differencing (see Template 5.3)
		case 4: // Grid Point Data - IEEE Floating Point Data (see Template 5.4)
			throw not_implemented();
			break;
		case 40: // Grid Point Data - JPEG2000 Compression (see Template 5.40)
			i.read(section.templ.gp_jpeg2000.R);
			i.read(section.templ.gp_jpeg2000.E);
			i.read(section.templ.gp_jpeg2000.D);
			i.read(section.templ.gp_jpeg2000.num_bits);
			i.read(section.templ.gp_jpeg2000.type_org);
			break;
		case 41: // Grid Point Data - PNG Compression (see Template 5.41)
			i.read(section.templ.gp_png.R);
			i.read(section.templ.gp_png.E);
			i.read(section.templ.gp_png.D);
			i.read(section.templ.gp_png.num_bits);
			i.read(section.templ.gp_png.type_org);
			break;
		case 50: // Spectral Data - Simple Packing (see Template 5.50)
			i.read(section.templ.sd_simple.R);
			i.read(section.templ.sd_simple.E);
			i.read(section.templ.sd_simple.D);
			i.read(section.templ.sd_simple.num_bits);
			i.read(section.templ.sd_simple.real);
			break;
		case 51: // Spectral Data - Complex Packing (see Template 5.51)
		case 61: // Grid Point Data - Simple Packing With Logarithm Pre-processing
		case 200: // Run Length Packing With Level Values (see Template 5.200)
			throw not_implemented();
			break;
		default: throw exception();
	}
}

static void unpack(std::istream & is, grib2::bitmap_section_t & section) throw (grib2::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw grib2::exception();

	grib2::octets::const_iterator i = buf.begin();

	i.read(section.bitmap_indicator); // see table 6.0

	// copy bitmap
	for (; i != buf.end(); ++i) {
		section.bitmap.push_back(*i);
	}
}

static void unpack(std::istream & is, data_section_t & section, const data_representation_section_t & drs) throw (grib2::exception)
{
	grib2::octets buf;
	grib2::octets::size_type length = section.length - sizeof(section.length) - sizeof(section.number);
	buf.reserve(length);
	if (buf.append(is, length) != length) throw grib2::exception();

	switch (drs.rep_templ) { // TODO; table 5.0
		case 0: // Grid Point Data - Simple Packing (see Template 5.0)
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
			throw not_implemented();
			break;
		default: throw exception();
	}
}

}

