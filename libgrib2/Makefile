# Makefile

.PHONY: all clean

CXX=g++
CXXFLAGS=-ggdb -Wall -Wextra -ansi -pedantic -I.

all : libgrib2.a

libgrib2.a : grib2_unpack.o
	ar rcs $@ $^

clean :
	rm -f *.o
	rm -f libgrib2.a

%.o : %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

