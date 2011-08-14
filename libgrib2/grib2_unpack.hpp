#ifndef __GRIB2_UNPACK__HPP__
#define __GRIB2_UNPACK__HPP__

#include <grib2.hpp>

namespace grib2 {

int unpack(message_t &, std::istream &);

}

#endif
