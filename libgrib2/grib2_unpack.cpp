#include <grib2_unpack.hpp>
#include <vector>

// http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc.shtml

namespace grib2 {

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
	if (search_next_message(is) < 0) return -1; // consumes "GRIB" indicator
	try {
		is >> grib.is;
/*
		is >> grib.ids;
		is >> grib.lus; // TODO: this section may be repeated
		is >> grib.gds; // TODO: this section may be repeated
*/
	} catch (exception &) {
		return -1;
	}
	return 0;
}

}

