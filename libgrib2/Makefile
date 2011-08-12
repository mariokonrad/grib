# Makefile

.PHONY: all clean

CC=gcc
CFLAGS=-ggdb -Wall -Wextra -ansi -pedantic -I. -I$(HOME)/tmp/grib_libraries/local/include

all : libgrib2.a

libgrib2.a :
	ar rcs $@ $^

clean :
	rm -f *.o
	rm -f libgrib2.a

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

