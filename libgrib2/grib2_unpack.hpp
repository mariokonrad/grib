#ifndef __GRIB2_UNPACK__HPP__
#define __GRIB2_UNPACK__HPP__

#include <grib2.hpp>
#include <istream>

namespace grib2 {

int grib2_unpack(Message &, std::istream &);

}

#endif
