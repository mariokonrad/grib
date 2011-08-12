#include <grib2_unpack.hpp>

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

int grib2_unpack(Message & grib, std::istream & is)
{
	grib.buffer.clear();
	grib.grids.clear();

	if (search_next_message(is) < 0) return -1;

	// TODO

	return -1;
}

}

