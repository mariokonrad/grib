# Makefile

.PHONY: all clean

CC=gcc
CFLAGS=-Wall -Wextra -ansi -pedantic -I. -I$(HOME)/tmp/grib_libraries/local/include

all : libgrib.a

libgrib.a : grib1_unpack.o grib2_unpack.o get_bits.o set_bits.o conv_float.o grib2_conv.o
	ar rcs $@ $^

clean :
	rm -f *.o
	rm -f libgrib.a

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

