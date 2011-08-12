# Makefile

.PHONY: all clean

CXX=g++
CXXFLAGS=-ggdb -Wall -Wextra -ansi -pedantic -I.

all : libgrib2.a

bittest : bittest.o bitset.hpp
	$(CXX) -o $@ $^

bittest.o : bittest.cpp bitset.hpp
	$(CXX) -o $@ -c bittest.cpp $(CXXFLAGS)

libgrib2.a : grib2_unpack.o
	ar rcs $@ $^

clean :
	rm -f *.o
	rm -f libgrib2.a
	rm -f *.exe *.stackdump

%.o : %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

