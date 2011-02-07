#include <get_bits.h>

/* Gets the contents of the various GRIB octets
 *
 * @param[in] buf GRIB buffer as a stream of bytes.
 * @param[out] loc The variable to hold the octet contents.
 * @param[in] off The offset in BITS from the beginning of the buffer to the beginning
 *     of the octet(s) to be unpacked.
 * @param[in] bits Number of BITS to unpack, will be a multiple of 8 since GRIB
 *     octets are 8 bits long.
 */
int get_bits(const unsigned char * buf, int * loc, size_t off, size_t bits)
{
	unsigned char bmask;
	int lmask;
	int temp;
	size_t buf_size = sizeof(unsigned char) * 8;
	size_t loc_size = sizeof(int) * 8;
	size_t wskip;
	int rshift;
	size_t n;

	/* no work to do */
	if (bits == 0) {
		return 0;
	}

	if (bits > loc_size) {
		fprintf(stderr,"Error: unpacking %d bits into a %d-bit field\n", bits, loc_size);
		return -1;
	} else {
		/* create masks to use when right-shifting (necessary because different
		   compilers do different things when right-shifting a signed bit-field) */
		bmask = 1;
		for (n = 1; n < buf_size; n++) {
			bmask <<= 1;
			bmask++;
		}
		lmask = 1;
		for (n = 1; n < loc_size; n++) {
			lmask <<= 1;
			lmask++;
		}

		/* get number of words to skip before unpacking begins */
		wskip = off / buf_size;

		/* right shift the bits in the packed buffer "word" to eliminate unneeded
		   bits */
		rshift = buf_size - (off % buf_size) - bits;

		/* check for a packed field spanning multiple "words" */
		if (rshift < 0) {
			*loc = 0;
			while (rshift < 0) {
				temp = buf[wskip++];
				*loc += (temp<<-rshift);
				rshift += buf_size;
			}
			if (rshift != 0) {
				*loc += (buf[wskip] >> rshift) &~ (bmask << (buf_size-rshift));
			} else {
				*loc += buf[wskip];
			}
		} else {
			*loc = (buf[wskip] >> rshift);
		}

		/* remove any unneeded leading bits */
		if (bits != loc_size) {
			*loc &= ~(lmask << bits);
		}
	}
	return 0;
}

