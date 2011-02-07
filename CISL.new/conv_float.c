#include <conv_float.h>
#include <get_bits.h>
#include <set_bits.h>
#include <math.h>

double ibm2real(unsigned char * buf, size_t off)
{
	int sign;
	int exp;
	int fr;
	double native_real;

	get_bits(buf, &sign, off, 1);
	get_bits(buf, &exp, off+1, 7);
	exp -= 64;
	get_bits(buf, &fr, off+8, 24);
	native_real = pow(2.0, -24.0) * (double)fr * pow(16.0, (double)exp);

	return (sign == 1) ? -native_real : native_real;
}

int ieee2ibm(double ieee)
{
	int ibm_real = 0;
	unsigned char * ir = (unsigned char *)&ibm_real;
	int sign = 0;
	int fr = 0;
	int exp = 64;
	const double full = 0xffffff;
	size_t size = sizeof(size_t) * 8;
	size_t off = 0;

	if (ieee != 0.0) {
		if (ieee < 0.0) {
			sign = 1;
			ieee = -ieee;
		}
		ieee /= pow(2.0, -24.0);
		while (exp > 0 && ieee < full) {
			ieee *= 16.0;
			exp--;
		}
		while (ieee > full) {
			ieee /= 16.0;
			exp++;
		}
		fr = ieee + 0.5;
		if (size > 32) {
			off = size - 32;
			set_bits(ir, 0, 0, off);
		}
		set_bits(ir, sign, off, 1);
		set_bits(ir, exp, off + 1, 7);
		set_bits(ir, fr, off + 8, 24);
	}
	return ibm_real;
}

