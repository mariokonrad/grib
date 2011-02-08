#include <set_bits.h>

/* Sets the contents of the various GRIB octets
 *
 * @param[out] buf GRIB buffer as stream of bytes.
 * @param[in] src Value of the octet to set.
 * @param[in] off Offset in bits from the beginning of the buffer to the beginning of the
 *     octet(s) to be packed.
 * @param[in] bits Number of bits to pack, will be a multiple of 8 since GRIB octets are 8 bits long.
 */
int set_bits(unsigned char * buf, int src, size_t off, size_t bits)
{
	unsigned char bmask;
	unsigned char left;
	unsigned char right;
	int smask;
	size_t buf_size = sizeof(unsigned char) * 8;
	size_t src_size = sizeof(int) * 8;
	size_t wskip;
	size_t bskip;
	size_t lclear;
	size_t rclear;
	size_t more;
	size_t n;

	/* no work to do */
	if (bits == 0) {
		return 0;
	}

	if (bits > src_size) {
		fprintf(stderr,"Error: packing %d bits from a %d-bit field\n",bits,src_size);
		return -1;
	} else {
		/* create masks to use when right-shifting (necessary because different
		   compilers do different things when right-shifting a signed bit-field) */
		bmask = 1;
		for (n = 1; n < buf_size; n++) {
			bmask <<= 1;
			bmask++;
		}
		smask = 1;
		for (n = 1; n < src_size; n++) {
			smask <<= 1;
			smask++;
		}
		/* get number of words and bits to skip before packing begins */
		wskip = off / buf_size;
		bskip = off % buf_size;
		lclear = bskip + bits;
		rclear = buf_size - bskip;
		left = (rclear != buf_size) ? (buf[wskip] & (bmask << rclear)) : 0;
		if (lclear <= buf_size) {
			/* all bits to be packed are in the current word; clear the field to be packed */
			right= (lclear != buf_size) ? (buf[wskip]&~(bmask<<(buf_size-lclear))) : 0;

			/* fill the field to be packed */
			buf[wskip] = (src_size != bits) ? src & ~(smask << bits) : src;
			buf[wskip] = left | right | (buf[wskip] << (rclear - bits));
		} else {
			/* bits to be packed cross a byte boundary(ies); clear the bit field to be packed */
			more = bits - rclear;
			buf[wskip] = left | ((src >> more) & ~(smask << (bits - more)));

			/* clear the next (or part of the next) word and pack those bits */
			while (more > buf_size) {
				more -= buf_size;
				buf[++wskip] = (src >> more) & ~(smask << (src_size - more));
			}
			wskip++;
			more = buf_size - more;
			right = (more != buf_size) ? (buf[wskip] & ~(bmask << more)) : 0;
			buf[wskip] = (buf_size > src_size) ? src & ~(bmask << src_size) : src;
			buf[wskip] = right | (buf[wskip] << more);
		}
	}
	return 0;
}

