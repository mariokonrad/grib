#include <grib1_write.h>
#include <bits.h>

int grib1_write_raw(unsigned char * buf, unsigned int len, int (*write_func)(const void *, unsigned int, void *), void * ptr)
{
	const char * HEAD = "GRIB";
	const char * TAIL = "7777";
	unsigned char tmp[3];

	if (write_func == NULL) {
		return -1;
	}

	if (write_func(HEAD, 4, ptr) != 4) {
		return -1;
	}

	set_bits(tmp, len + 12, 0, 24);
	if (write_func(tmp, 3, ptr) != 3) {
		return -1;
	}

	tmp[0] = 1;
	if (write_func(tmp, 1, ptr) != 1) {
		return -1;
	}

	if (write_func(buf, len, ptr) != len) {
		return -1;
	}

	if (write_func(TAIL, 4, ptr) != 4) {
		return -1;
	}

	return 0;
}

int grib1_write(GRIBRecord * grib, int (*write_func)(const void *, unsigned int, void *), void * ptr)
{
	if (grib == NULL) {
		return -1;
	}
	if (write_func == NULL) {
		return -1;
	}

	/* TODO
	return write_func(grib->buffer, ???, ptr);
	*/
	return 0;
}

