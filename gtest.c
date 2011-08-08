#include <g2clib/grib2.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv)
{
	unsigned char *cgrib;
	g2int  listsec0[3],listsec1[13],numlocal,numfields;
	long   lskip,n,lgrib,iseek;
	int    unpack,ret,ierr,expand;
	gribfield  *gfld;
	FILE   *fptr;
	size_t  lengrib;
	int cnt = 0;

	iseek=0;
	unpack=1;
	expand=1;
	fptr=fopen(argv[1],"r");
	for (;;) {
		seekgb(fptr,iseek,32000,&lskip,&lgrib);
		if (lgrib == 0) break;    /* end loop at EOF or problem */
		cgrib=(unsigned char *)malloc(lgrib);
		ret=fseek(fptr,lskip,SEEK_SET);
		lengrib=fread(cgrib,sizeof(unsigned char),lgrib,fptr);
		iseek=lskip+lgrib;
		ierr=g2_info(cgrib,listsec0,listsec1,&numfields,&numlocal);
		if (ierr != 0) {
			free(cgrib);
			printf("Error in reading field\n");
			exit(-1);
		}
		for (n=0;n<numfields;n++) {
			ierr=g2_getfld(cgrib,n+1,unpack,expand,&gfld);
			if (ierr != 0) {
				g2_free(gfld);
				printf("Error in reading field\n");
				exit(-1);
			}
			printf("GRID: #points  = %d\n", gfld->ngrdpts);
			printf("      #datapts = %d\n", gfld->ndpts);
/*
			.
			.  // Process Field Here
			.
*/
			g2_free(gfld);
		}
		free(cgrib);
	}
	return 0;
}

