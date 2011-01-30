#include <stdio.h>
#include <stdlib.h>
#include "unpackgrib2.c"

/*  If the compiler complains about the "pow" function being an undefined
**  symbol, recompile like this:
**    % cc grib2_access.c -lm
*/

int main(int argc,char **argv)
{
  GRIBMessage grib_msg;
  FILE *fp;
  size_t n,m,l,nmsg=0,x;
  int status;
  int hr,min,sec;

  grib_msg.buffer=NULL;
  grib_msg.grids=NULL;

  if (argc != 2) {
    fprintf(stderr,"usage: %s GRIB2_file_name\n",argv[0]);
    exit(1);
  }

  fp=fopen(argv[1],"rb");

  while ( (status=unpackgrib2(fp,&grib_msg)) == 0) {
    nmsg++;

/* print some header information */
    hr=grib_msg.time/10000;
    min=(grib_msg.time/100) % 100;
    sec=grib_msg.time % 10000;
    printf("Message Number: %d  GRIB Edition: %d  Discipline: %d  Table Version: %d-%d  Source ID: %d-%d  Date: %4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d  Number of Grids: %d\n",nmsg,grib_msg.ed_num,grib_msg.disc,grib_msg.table_ver,grib_msg.local_table_ver,grib_msg.center_id,grib_msg.sub_center_id,grib_msg.yr,grib_msg.mo,grib_msg.dy,hr,min,sec,grib_msg.num_grids);
    for (l=0; l < grib_msg.num_grids; l++) {
	printf("  Grid #: %d  Type: %d  Parameter: %d/%d  Level: %d/%f  R: %f\n",l+1,grib_msg.grids[l].md.gds_templ_num,grib_msg.grids[l].md.param_cat,grib_msg.grids[l].md.param_num,grib_msg.grids[l].md.lvl1_type,grib_msg.grids[l].md.lvl1,grib_msg.grids[l].md.R);
	printf("    Definition:  Dimensions: %d x %d  ",grib_msg.md.nx,grib_msg.md.ny);
	if (grib_msg.md.scan_mode == 0)
	  printf("NW Corner (Lat/Lon): %f,%f",grib_msg.md.slat,grib_msg.md.slon);
	else if (grib_msg.md.scan_mode == 0x40)
	  printf("NW Corner (Lat/Lon): %f,%f",grib_msg.md.lats.elat,grib_msg.md.slon);
	switch (grib_msg.md.gds_templ_num) {
	  case 0:
	  case 40:
	    if (grib_msg.md.scan_mode == 0)
		printf("  SE Corner (Lat/Lon): %f,%f",grib_msg.md.lats.elat,grib_msg.md.lons.elon);
	    else if (grib_msg.md.scan_mode == 0x40)
		printf("  SE Corner (Lat/Lon): %f,%f",grib_msg.md.slat,grib_msg.md.lons.elon);
	    if (grib_msg.md.gds_templ_num == 0)
	      printf("  Lat/Lon Resolution: %f,%f\n",grib_msg.md.yinc.lainc,grib_msg.md.xinc.loinc);
	    else if (grib_msg.md.gds_templ_num == 40)
	      printf("  Lat Circles %d, Lon Resolution: %f\n",grib_msg.md.yinc.lainc,grib_msg.md.xinc.loinc);
	    break;
	}
/* print out the gridpoints for the grids in the first message */
	if (nmsg == 1) {
	  x=0;
	  for (n=0; n < grib_msg.md.ny; n++) {
	    for (m=0; m < grib_msg.md.nx; m++) {
		printf("(i,j)=(%d,%d)",m,n);
		if (grib_msg.grids[l].gridpoints[x] == GRIB_MISSING_VALUE)
		  printf(" value=MISSING\n");
		else
		  printf(" value=%f\n",grib_msg.grids[l].gridpoints[x]);
		x++;
	    }
	  }
	}
    }
  }
  if (status == -1)
    printf("EOF - end of file found\n");
  else
    printf("Read error after %d messages\n",nmsg);
}
