#include <grib2.hpp>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

static void unexpected()
{
	std::cerr << "UNEXPECTED TERMINATION" << std::endl;
	exit(-1);
}

int main(int, char ** argv)
{
	std::set_unexpected(unexpected);

	std::ifstream ifs(argv[1], std::ifstream::in | std::ifstream::binary);

	while (!ifs.eof() && ifs.good()) {
		grib2::message_t grib;
		memset(&grib, 0, sizeof(grib));
		grib2::unpack(grib, ifs);
	}

	return 0;
}

