# Makefile

.PHONY: all clean

CC=gcc
CFLAGS=-Wall -Wextra -ansi -pedantic -I. -I$(HOME)/tmp/grib_libraries/local/include

LIB_JASPER=-L$(HOME)/tmp/grib_libraries/local/lib -ljasper

all : libgrib.a grib2_to_grib1

libgrib.a : grib1_unpack.o grib2_unpack.o get_bits.o set_bits.o conv_float.o grib2_conv.o
	ar rcs $@ $^

grib2_to_grib1 : grib2_to_grib1.o libgrib.a
	$(CC) -o $@ grib2_to_grib1.o -L. -lgrib -lm $(LIB_JASPER)

clean :
	rm -f *.o
	rm -f libgrib.a
	rm -f grib2_to_grib1

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

