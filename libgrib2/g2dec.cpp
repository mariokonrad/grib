#include <grib2.hpp>
#include <fstream>

int main(int, char ** argv)
{
	std::ifstream ifs(argv[1], std::ifstream::in | std::ifstream::binary);
	grib2::message_t grib;

	while (!ifs.eof() && ifs.good()) grib2::unpack(grib, ifs);

	return 0;
}

