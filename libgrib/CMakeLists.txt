cmake_minimum_required(VERSION 2.6)

project(grib)

#find_package(jasper)

#if (JASPER_INCLUDE_DIR)
#	message(STATUS "Jasper has been found")
#else (JASPER_INCLUDE_DIR)
#	set(JASPER_INCLUDE_DIR NOTFOUND)
#	set(JASPER_LIBRARY NOTFOUND)
#	message(STATUS "Jasper is required")
#endif (JASPER_INCLUDE_DIR)

include_directories(
	.
	)

add_library(
	grib STATIC
	get_bits.c
	set_bits.c
	conv_float.c
	grib1_unpack.c
	grib2_unpack.c
	grib2_conv.c
	)

