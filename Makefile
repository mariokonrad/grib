.PHONY: all clean

CURL_INCLUDE=`curl-config --cflags`
CURL_LIB=`curl-config --static-libs`

CXX=g++
CC=gcc

CXXFLAGS=-Wall -Wextra -ansi -pedantic -O2
CFLAGS=-Wall -Wextra -ansi -pedantic -O2

all : grib wgrib

grib : grib.o
	$(CXX) -o $@ $^ $(CURL_LIB)

wgrib : wgrib.o
	$(CC) -o $@ $^

#grib2decode : grib2decode.o
#	$(CXX) -o $@ $^ -L../grib_libraries/g2clib-1.2.1 -lg2c -L../grib_libraries/local/lib -ljasper -lpng

#grib2decode.o : grib2decode.cpp
#	$(CXX) -o $@ -c $< $(CXXFLAGS) -I../grib_libraries/g2clib-1.2.1

clean :
	rm -f grib
	rm -f wgrib
	rm -f grib2decode
	rm -f *.o

%.o : %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(CURL_INCLUDE)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

