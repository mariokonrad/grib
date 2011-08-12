#include <grib2_unpack.hpp>
#include <fstream>

int main(int, char ** argv)
{
	std::ifstream ifs(argv[1]);
	grib2::message_t grib;

	grib2::grib2_unpack(grib, ifs);

	return 0;
}

