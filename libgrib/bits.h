#ifndef __BITS__H__
#define __BITS__H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned char * buffer; /* the buffer carrying data, ASSUMPTION: sizeof(unsigned char)==1 */
	unsigned int length; /* size of allocated buffer in bytes */

	unsigned int offset; /* the write offset within the buffer, measured in bits */
} buffer_t;

int buffer_alloc(buffer_t * buf, unsigned int length);
void buffer_free(buffer_t * buf);

int get_bits(const unsigned char * buf, int * loc, size_t off, size_t bits);
int set_bits(unsigned char *buf, int src, size_t off, size_t bits);
int append_bits(buffer_t * buf, int src, size_t bits);

#ifdef __cplusplus
}
#endif

#endif
