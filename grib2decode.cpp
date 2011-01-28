#include <cstdio>

extern "C" {
#include <grib2.h>
}

int main(int, char **)
{
	unsigned char * cgrib;
	g2int listsec0[3];
	g2int listsec1[13];
	g2int numlocal;
	g2int numfields;
	long lskip;
	long lgrib;
	long iseek = 0;
	int unpack = 1;
	int expand = 1;
	int ret;
	gribfield * gfld;
	FILE * file = NULL;
	size_t lengrib;

	file = fopen("test.grb2", "r");
	if (file == NULL) {
		printf("ERROR: cannot open file. exit.\n");
		return -1;
	}

	for (;;) {
		seekgb(file, iseek, 32000, &lskip, &lgrib);
		if (lgrib == 0) {
			// end loop at EOF or error occurred
			break;
		}
		cgrib = new unsigned char [lgrib];
		if (cgrib == NULL) {
			printf("ERROR: cannot allocate memory. exit.\n");
			break;
		}
		ret = fseek(file, lskip, SEEK_SET);
		lengrib = fread(cgrib, sizeof(unsigned char), lgrib, file);
		iseek = lskip + lgrib;
		int err = g2_info(cgrib, listsec0, listsec1, &numfields, &numlocal);

		for (int n = 0; n < numfields; ++n) {
			err = g2_getfld(cgrib, n+1, unpack, expand, &gfld);

			printf("\n");
			printf("version   : %d\n", gfld->version);
			printf("discipline: %d\n", gfld->discipline);
			printf("idsect\n");
			for (int i = 0; i < gfld->idsectlen; ++i) printf("        %2d: %d\n", i, gfld->idsect[i]);
			printf("idsectlen : %d\n", gfld->idsectlen);
			printf("local     : %p\n", gfld->local);
			printf("locallen  : %d\n", gfld->locallen);
			printf("ifldnum   : %d\n", gfld->ifldnum);
			printf("griddef   : %d\n", gfld->griddef);
			printf("ngrdpts   : %d\n", gfld->ngrdpts);
			printf("numoct_opt: %d\n", gfld->numoct_opt);
			printf("interp_opt: %d\n", gfld->interp_opt);
			printf("num_opt   : %d\n", gfld->num_opt);
			printf("list_opt  : %p\n", gfld->list_opt);
			printf("igdtnum   : %d\n", gfld->igdtnum);
			printf("igdtlen   : %d\n", gfld->igdtlen);
			printf("igdtmpl   : %p\n", gfld->igdtmpl);
			printf("ipdtnum   : %d\n", gfld->ipdtnum);
			printf("ipdtlen   : %d\n", gfld->ipdtlen);
			printf("ipdtmpl   : %p\n", gfld->ipdtmpl);
			printf("num_coord : %d\n", gfld->num_coord);
			printf("coord_list: %p\n", gfld->coord_list);
			printf("ndpts     : %d\n", gfld->ndpts);
			printf("idrtnum   : %d\n", gfld->idrtnum);
			printf("idrtlen   : %d\n", gfld->idrtlen);
			printf("idrtmpl   : %p\n", gfld->idrtmpl);
			printf("unpacked  : %d\n", gfld->unpacked);
			printf("expanded  : %d\n", gfld->expanded);
			printf("ibmap     : %d\n", gfld->ibmap);
			printf("bmap      : %p\n", gfld->bmap);
			printf("fld       : %p\n", gfld->fld);

			g2_free(gfld);
		}

		delete [] cgrib;
	}

	fclose(file);
	return 0;
}

