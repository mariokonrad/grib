#include <stdio.h>
#include <grib2.h>
#include <grib2_conv.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static FILE * ofp = NULL;

typedef struct {
	unsigned char * data;
	unsigned int len;
	unsigned int ofs;
} data_buffer_t;

static data_buffer_t buf_src = { NULL, 0, 0 };
static data_buffer_t buf_dst = { NULL, 0, 0 };

static int read_func(void * buf, unsigned int len)
{
	if (buf == NULL || buf_src.data == NULL || len == 0) return 0;
	if (buf_src.ofs >= buf_src.len) return 0;
	
	if (buf_src.ofs + len > buf_src.len) {
		len = buf_src.len - buf_src.ofs + 1;
		memcpy(buf, buf_src.data + buf_src.ofs, len);
		buf_src.ofs = buf_src.len;
	} else {
		memcpy(buf, buf_src.data + buf_src.ofs, len);
		buf_src.ofs += len;
	}	
	return len;
}

static int write_func(const void * buf, unsigned int len)
{
	if (buf == NULL || len == 0) return 0;

	if (buf_dst.data == NULL) {
		buf_dst.data = (unsigned char *)malloc(len);
		buf_dst.len = len;
		buf_dst.ofs = 0;
	}

	if (buf_dst.ofs + len > buf_dst.len) {
		buf_dst.len = buf_dst.ofs + len;
		buf_dst.data = (unsigned char *)realloc(buf_dst.data, buf_dst.len);
	}

	memcpy(buf_dst.data + buf_dst.ofs, buf, len);
	buf_dst.ofs += len;

	return len;
}

int main(int argc, char ** argv)
{
	FILE * ifp = NULL;
	struct stat s;

	if (argc != 3) {
		fprintf(stderr, "usage: %s GRIB2_file_name GRIB1_file_name\n", argv[0]);
		return -1;
	}

	if (stat(argv[1], &s)) {
		fprintf(stderr, "%s: error: cannot read file info of '%s'. exit.\n", argv[0], argv[1]);
		return -1;
	}

	ifp = fopen(argv[1], "rb");
	if (ifp == NULL) {
		fprintf(stderr, "%s: error: cannot open file '%s'. exit.\n", argv[0], argv[1]);
		return -1;
	}

	buf_src.len = s.st_size;
	buf_src.data = (unsigned char *)malloc(buf_src.len);
	if (fread(buf_src.data, sizeof(unsigned char), buf_src.len, ifp) != buf_src.len) {
		fprintf(stderr, "%s: error while reading file '%s'. exit.\n", argv[0], argv[1]);
		return -1;
	}
	fclose(ifp);
	
	grib2_to_grib1_conv(read_func, write_func);

	ofp = fopen(argv[2], "wb");
	if (ofp == NULL) {
		fprintf(stderr, "%s: error: cannot open file '%s'. exit.\n", argv[0], argv[2]);
		return -1;
	}
	if (fwrite(buf_dst.data, sizeof(unsigned char), buf_dst.len, ofp) != buf_dst.len) {
		fprintf(stderr, "%s: error while writing file '%s'. exit.\n", argv[0], argv[2]);
		return -1;
	}
	fclose(ofp);

	free(buf_src.data);
	free(buf_dst.data);

	printf("GRIB2 data: %u\n", buf_src.len);
	printf("GRIB1 data: %u\n", buf_dst.len);

	return 0;
}

