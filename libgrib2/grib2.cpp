#include <grib2.hpp>
#include <bitset.hpp>

std::istream & operator>>(std::istream & is, grib2::indicator_section_t & section) throw (grib2::exception)
{
	uint16_t reserved;
	bitset<uint8_t> b;

	b.reserve(128);
	if (b.append(is, 12) != 12) throw grib2::exception();
	bitset<uint8_t>::const_iterator i = b.begin();
	i.read(reserved);
	i.read(section.discipline);
	i.read(section.edition);
	i.read(section.length);

	return is;
}

std::istream & operator>>(std::istream & is, grib2::identification_section_t & section) throw (grib2::exception)
{
	bitset<uint8_t> b;

	b.reserve(1024);

	if (b.append(is, sizeof(section.length)) != sizeof(section.length)) throw grib2::exception();
	bitset<uint8_t>::const_iterator i = b.begin();
	i.read(section.length);
	b.clear();

	if (b.append(is, section.length - sizeof(section.length) != section.length - sizeof(section.length)))
		throw grib2::exception();
	i = b.begin();

	i.read(section.number);
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

	return is;
}

std::istream & operator>>(std::istream & is, grib2::local_use_section_t & section) throw (grib2::exception)
{
	bitset<uint8_t> b;

	b.reserve(64);

	if (b.append(is, sizeof(section.length)) != sizeof(section.length)) throw grib2::exception();
	bitset<uint8_t>::const_iterator i = b.begin();
	i.read(section.length);
	b.clear();

	if (b.append(is, section.length - sizeof(section.length) != section.length - sizeof(section.length)))
		throw grib2::exception();
	i = b.begin();

	i.read(section.number);

	// all additional data is for local use and ignored here

	return is;
}

std::istream & operator>>(std::istream & is, grib2::grid_definition_section_t & section) throw (grib2::exception)
{
	bitset<uint8_t> b;

	b.reserve(1024);
	if (b.append(is, sizeof(section.length)) != sizeof(section.length)) throw grib2::exception();
	bitset<uint8_t>::const_iterator i = b.begin();
	i.read(section.length);
	b.clear();

	if (b.append(is, section.length - sizeof(section.length) != section.length - sizeof(section.length)))
		throw grib2::exception();
	i = b.begin();

	i.read(section.number);
	i.read(section.source);
	i.read(section.num_datapoints);
	i.read(section.num_optional);
	i.read(section.interpol_list);
	i.read(section.grid_def_templ);

	// TODO
	switch (section.grid_def_templ) {
		case 0: // Latitude/Longitude (template 3.0)
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
			break;
	}

	// TODO

	return is;
}

