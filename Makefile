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

clean :
	rm -f grib
	rm -f wgrib
	rm -f *.o

%.o : %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(CURL_INCLUDE)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

