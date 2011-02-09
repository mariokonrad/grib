#include <conv_float.h>
#include <bits.h>
#include <math.h>

/* IBM 32bit single precision
 * sAAAAAAA BBBBBBBB BBBBBBBB BBBBBBBB
 * R = (-1)^s * 2^(-24) * B * 16^(A-64)
 */

double ibm2real(const unsigned char * buf, unsigned int off)
{
	int s;
	int e;
	int b;

	get_bits(buf, &s, off + 0,  1);
	get_bits(buf, &e, off + 1,  7);
	get_bits(buf, &b, off + 8, 24);
	return pow(2.0, -24.0) * (double)(s ? -b : b) * pow(16.0, (double)e - 64);
}

int32_t ieee2ibm(double ieee)
{
	int32_t ibm_real = 0;
	unsigned char * ir = (unsigned char *)&ibm_real;

	int s = 0;
	int b = 0;
	int e = 64;
	const double full = 0xffffff;
	unsigned int off = 0;

	if (ieee != 0.0) {
		if (ieee < 0.0) {
			s = 1;
			ieee = -ieee;
		}
		ieee /= pow(2.0, -24.0);
		while (e > 0 && ieee < full) {
			ieee *= 16.0;
			e--;
		}
		while (ieee > full) {
			ieee /= 16.0;
			e++;
		}
		b = ieee + 0.5;

		/*
		if (sizeof(ibm_real)*8 > 32) {
			off = sizeof(ibm_real)*8 - 32;
			set_bits(ir, 0, 0, off);
		}
		*/

		set_bits(ir, s, off + 0,  1);
		set_bits(ir, e, off + 1,  7);
		set_bits(ir, b, off + 8, 24);
	}
	return ibm_real;
}

