#ifndef __BITS__H__
#define __BITS__H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned char * buffer; /* the buffer carrying data, ASSUMPTION: sizeof(unsigned char)==8 */
	unsigned int offset; /* the write offset within the buffer, measured in bits */
} Buffer;

int get_bits(const unsigned char * buf, int * loc, size_t off, size_t bits);
int set_bits(unsigned char *buf, int src, size_t off, size_t bits);
int append_bits(Buffer * buf, int src, size_t bits);

#ifdef __cplusplus
}
#endif

#endif
