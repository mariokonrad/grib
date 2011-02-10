#include <stdio.h>
#include <grib2.h>
#include <grib2_conv.h>
#include <bits.h>
#include <math.h>
#include <stdlib.h>

static FILE * ifp = NULL;
static FILE * ofp = NULL;

static int read_func(void * buf, unsigned int len)
{
	return ifp == NULL
		? 0
		: fread(buf, 1, len, ifp);
}

static int write_func(const void * buf, unsigned int len)
{
	return ofp == NULL
		? 0
		: fwrite(buf, 1, len, ofp);
}

int main(int argc, char ** argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: %s GRIB2_file_name GRIB1_file_name\n", argv[0]);
		return -1;
	}

	ifp = fopen(argv[1], "rb");
	ofp = fopen(argv[2], "wb");
	grib2_to_grib1_conv(read_func, write_func);
	fclose(ifp);
	fclose(ofp);

	return 0;
}

