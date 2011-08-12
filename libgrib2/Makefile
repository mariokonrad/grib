# Makefile

.PHONY: all clean

CXX=g++
CXXFLAGS=-ggdb -Wall -Wextra -ansi -pedantic -I.

all : g2dec libgrib2.a

bittest : bittest.o
	$(CXX) -o $@ $^

bittest.o : bittest.cpp bitset.hpp
	$(CXX) -o $@ -c bittest.cpp $(CXXFLAGS)

g2dec : g2dec.o libgrib2.a
	$(CXX) -o $@ g2dec.o -L. -lgrib2

g2dec.o : g2dec.cpp
	$(CXX) -o $@ -c g2dec.cpp $(CXXFLAGS)

libgrib2.a : grib2_unpack.o grib2.o
	ar rcs $@ $^

clean :
	rm -f *.o
	rm -f libgrib2.a
	rm -f *.exe *.stackdump

%.o : %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

