.PHONY: all clean

LIB_JASPER=-L$(HOME)/tmp/grib_libraries/local/lib -ljasper

CURL_INCLUDE=`curl-config --cflags`
CURL_LIB=`curl-config --static-libs`

CXX=g++
CC=gcc

CXXFLAGS=-Wall -Wextra -ansi -pedantic -ggdb
CFLAGS=-Wall -Wextra -ansi -pedantic -ggdb -Ilibgrib

all : grib wgrib grib2_to_grib1 grib2_to_grib1_mem

grib : libgrib/libgrib.a grib.o
	$(CXX) -o $@ grib.o $(CURL_LIB) -Llibgrib -lgrib -lm $(LIB_JASPER)

grib.o : grib.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(CURL_INCLUDE) -Ilibgrib

libgrib/libgrib.a :
	$(MAKE) -C libgrib

wgrib : wgrib.o
	$(CC) -o $@ $^

grib2_to_grib1 : grib2_to_grib1.o
	$(CC) -o $@ grib2_to_grib1.o -Llibgrib -lgrib -lm $(LIB_JASPER)

grib2_to_grib1_mem : grib2_to_grib1_mem.o
	$(CC) -o $@ grib2_to_grib1_mem.o -Llibgrib -lgrib -lm $(LIB_JASPER)

#grib2decode : grib2decode.o
#	$(CXX) -o $@ $^ -L../grib_libraries/g2clib-1.2.1 -lg2c -L../grib_libraries/local/lib -ljasper -lpng

#grib2decode.o : grib2decode.cpp
#	$(CXX) -o $@ -c $< $(CXXFLAGS) -I../grib_libraries/g2clib-1.2.1

clean :
	rm -f *.o grib wgrib grib2decode grib2_to_grib1 grib2_to_grib1_mem
	$(MAKE) -C libgrib clean

%.o : %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(CURL_INCLUDE)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

