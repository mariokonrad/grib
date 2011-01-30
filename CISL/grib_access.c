#include <stdio.h>
#include <stdlib.h>
#include "unpackgrib.c"

/*  If the compiler complains about the "pow" function being an undefined
**  symbol, recompile like this:
**    % cc grib_access.c -lm
**
**  For little-endian systems, such as PCs, compile with the "-Dlinux" option
*/

int main(int argc,char **argv)
{
  GRIBRecord grib_rec;
  FILE *fp;
  size_t n,m,nrec=0;
  int status;
  int hr,min;

  grib_rec.buffer=NULL;
  grib_rec.pds_ext=NULL;
  grib_rec.gridpoints=NULL;

  if (argc != 2) {
    fprintf(stderr,"usage: %s GRIB_file_name\n",argv[0]);
    exit(1);
  }

  fp=fopen(argv[1],"rb");

  while ( (status=unpackgrib(fp,&grib_rec)) == 0) {
    nrec++;

/* print some header information */
    hr=grib_rec.time/100;
    min=grib_rec.time % 100;
    printf("Record Number: %d  GRIB Edition: %d  GRIB Parameter Table Version: %d  Source ID: %d-%d  Grid Type: %d  Date: %4.4d-%2.2d-%2.2d %2.2d:%2.2d  Parameter Code: %d  Level(s): %d/%d,%d  Reference Value: %f\n",nrec,grib_rec.ed_num,grib_rec.table_ver,grib_rec.center_id,grib_rec.sub_center_id,grib_rec.grid_type,grib_rec.yr,grib_rec.mo,grib_rec.dy,hr,min,grib_rec.param,grib_rec.level_type,grib_rec.lvl1,grib_rec.lvl2,grib_rec.ref_val);
    printf("Grid Definition:  Dimensions: %d x %d  Bottom/Left Corner (Lat/Lon): %f,%f",grib_rec.nx,grib_rec.ny,grib_rec.slat,grib_rec.slon);
    switch (grib_rec.data_rep) {
	case 0:
	  printf("  Top/Right Corner (Lat/Lon): %f,%f  Lat/Lon Resolution: %f,%f\n",grib_rec.elat,grib_rec.elon,grib_rec.lainc,grib_rec.loinc);
	  break;
	case 4:
	  printf("  Top/Right Corner (Lat/Lon): %f,%f  Number of Latitude Circles: %d  Longitude Resolution: %f\n",grib_rec.elat,grib_rec.elon,grib_rec.lainc,grib_rec.loinc);
	  break;
	case 5:
	  printf("  X/Y Resolution (meters): %d,%d\n",grib_rec.xlen,grib_rec.ylen);
	  break;
    }

/* print the gridpoints for the first record */
    if (nrec == 1) {
	for (n=0; n < grib_rec.ny; n++) {
	  for (m=0; m < grib_rec.nx; m++) {
	    printf("(i,j)=(%d,%d)",m,n);
	    if (grib_rec.gridpoints[n][m] == GRIB_MISSING_VALUE)
		printf(" value=MISSING\n",n,m);
	    else
		printf(" value=%f\n",grib_rec.gridpoints[n][m]);
	  }
	}
    }
  }
  if (status == -1)
    printf("EOF - end of file found\n");
  else
    printf("Read error after %d records\n",nrec);
}
