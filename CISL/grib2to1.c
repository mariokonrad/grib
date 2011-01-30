/*
** File: grib2to1.c
**
** Author:  Bob Dattore
**          NCAR/DSS
**          dattore@ucar.edu
**          (303) 497-1825
**
** Purpose: to provide a simple C program for converting from GRIB2 to GRIB1
**
** This program has been tested on Sun/Solaris and Dell/Windows XP systems.
**
** Revision History:
**          20 Feb 2008 - first version; will convert analyses and forecasts
**            on a regular latitude/longitude grid
**          31 Mar 2008 - added code for Lambert conformal grids (GDS template
**            3.30), statistically processed data (PDS template 4.8), and some
**            NCEP-specific parameter mappings
**          03 Apr 2008 - bug fixes
**          17 Apr 2008 - change reference value packing so that it works on
**            little-endian machines
**          16 Jul 2008 - added more GRIB2 to GRIB1 parameter mappings, added
**            PDS packing for GRIB2 Product Definition Template 4.2
**          22 Jul 2008 - patch for NCEP max/min temperature grids
**          03 May 2010 - patch for NCEP CFSR monthly grids
**
** Contact Bob Dattore at dattore@ucar.edu to get conversions for other products
** and grid definitions added.
**
** You will need to download the GRIB2 decoder:
**    http://dss.ucar.edu/libraries/grib/c_routines/unpackgrib2.c
** It must be in the same directory as this program.
**
** You will also need to link with the JasPer library, which needs the JPEG-6b
** library.
**     You can get the JasPer code at
**       http://www.ece.uvic.ca/~mdadams/jasper/
**     You can get the installation code for libjpeg.a at
**       http://www.ijg.org/files/jpegsrc.v6b.tar.gz
**
** Example compile command:
**    % cc -o grib2to1 -I<jasper include path> -L<jasper library path> grib2to1.c -ljasper
**      where <jasper include path> is the directory path of the jasper header
**            files
**            <jasper library path> is the directory path of the jasper library
**
** If the compiler complains about the "pow" function being an undefined
** symbol, include the math library in the compile (-lm)
**
** To use the program:
**    % grib2to1 <name of GRIB2 file to convert> <name of GRIB1 file to create>
*/

#include <stdio.h>
#include <stdlib.h>
#include "unpackgrib2.c"

/* setBits sets the contents of the various GRIB octets
**   buf is the GRIB buffer as a stream of bytes
**   src is the value of the octet(s) to set
**   off is the offset in BITS from the beginning of the buffer to the beginning
**       of the octet(s) to be packed
**   bits is the number of BITS to pack - will be a multiple of 8 since GRIB
**       octets are 8 bits long
*/
void setBits(unsigned char *buf,int src,size_t off,size_t bits)
{
  unsigned char bmask,left,right;
  int smask;
  size_t buf_size=sizeof(unsigned char)*8,src_size=sizeof(int)*8;
  size_t wskip,bskip,lclear,rclear,more;
  size_t n;

// no work to do
  if (bits == 0)
    return;

  if (bits > src_size) {
    fprintf(stderr,"Error: packing %d bits from a %d-bit field\n",bits,src_size);
    exit(1);
  }
  else {
// create masks to use when right-shifting (necessary because different
// compilers do different things when right-shifting a signed bit-field)
    bmask=1;
    for (n=1; n < buf_size; n++) {
      bmask<<=1;
      bmask++;
    }
    smask=1;
    for (n=1; n < src_size; n++) {
	smask<<=1;
	smask++;
    }
// get number of words and bits to skip before packing begins
    wskip=off/buf_size;
    bskip=off % buf_size;
    lclear=bskip+bits;
    rclear=buf_size-bskip;
    left= (rclear != buf_size) ? (buf[wskip]&(bmask<<rclear)) : 0;
    if (lclear <= buf_size) {
// all bits to be packed are in the current word; clear the field to be
// packed
	right= (lclear != buf_size) ? (buf[wskip]&~(bmask<<(buf_size-lclear))) : 0;
// fill the field to be packed
	buf[wskip]= (src_size != bits) ? src&~(smask<<bits) : src;
	buf[wskip]=left|right|(buf[wskip]<<(rclear-bits));
    }
    else {
// bits to be packed cross a byte boundary(ies); clear the bit field to be
// packed
	more=bits-rclear;
	buf[wskip]=left|((src>>more)&~(smask<<(bits-more)));
// clear the next (or part of the next) word and pack those bits
	while (more > buf_size) {
	  more-=buf_size;
	  buf[++wskip]=(src>>more)&~(smask<<(src_size-more));
	}
	wskip++;
	more=buf_size-more;
	right= (more != buf_size) ? (buf[wskip]&~(bmask<<more)) : 0;
	buf[wskip]= (buf_size > src_size) ? src&~(bmask<<src_size) : src;
	buf[wskip]=right|(buf[wskip]<<more);
    }
  }
}

int mapParameterData(int center,int disc,int param_cat,int param_num)
{
  switch (disc) {
// meteorological products
    case 0:
	switch (param_cat) {
// temperature parameters
	  case 0:
	    switch (param_num) {
		case 0:
		  return 11;
		case 1:
		  return 12;
		case 2:
		  return 13;
		case 3:
		  return 14;
		case 4:
		  return 15;
		case 5:
		  return 16;
		case 6:
		  return 17;
		case 7:
		  return 18;
		case 8:
		  return 19;
		case 9:
		  return 25;
		case 10:
		  return 121;
		case 11:
		  return 122;
		case 12:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Heat index'\n");
		  exit(1);
		case 13:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Wind chill factor'\n");
		  exit(1);
		case 14:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Minimum dew point depression'\n");
		  exit(1);
		case 15:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Virtual potential temperature'\n");
		  exit(1);
		case 16:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Snow phase change heat flux'\n");
		  exit(1);
		case 192:
		  switch (center) {
		    case 7:
			return 229;
		  }
	    }
	    break;
// moisture parameters
	  case 1:
	    switch (param_num) {
		case 0:
		  return 51;
		case 1:
		  return 52;
		case 2:
		  return 53;
		case 3:
		  return 54;
		case 4:
		  return 55;
		case 5:
		  return 56;
		case 6:
		  return 57;
		case 7:
		  return 59;
		case 8:
		  return 61;
		case 9:
		  return 62;
		case 10:
		  return 63;
		case 11:
		  return 66;
		case 12:
		  return 64;
		case 13:
		  return 65;
		case 14:
		  return 78;
		case 15:
		  return 79;
		case 16:
		  return 99;
		case 17:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Snow age'\n");
		  exit(1);
		case 18:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Absolute humidity'\n");
		  exit(1);
		case 19:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Precipitation type'\n");
		  exit(1);
		case 20:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Integrated liquid water'\n");
		  exit(1);
		case 21:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Condensate water'\n");
		  exit(1);
		case 22:
		  switch (center) {
		    case 7:
			return 153;
		  }
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Cloud mixing ratio'\n");
		  exit(1);
		case 23:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Ice water mixing ratio'\n");
		  exit(1);
		case 24:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Rain mixing ratio'\n");
		  exit(1);
		case 25:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Snow mixing ratio'\n");
		  exit(1);
		case 26:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Horizontal moisture convergence'\n");
		  exit(1);
		case 27:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Maximum relative humidity'\n");
		  exit(1);
		case 28:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Maximum absolute humidity'\n");
		  exit(1);
		case 29:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Total snowfall'\n");
		  exit(1);
		case 30:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Precipitable water category'\n");
		  exit(1);
		case 31:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Hail'\n");
		  exit(1);
		case 32:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Graupel (snow pellets)'\n");
		  exit(1);
		case 33:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Categorical rain'\n");
		  exit(1);
		case 34:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Categorical freezing rain'\n");
		  exit(1);
		case 35:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Categorical ice pellets'\n");
		  exit(1);
		case 36:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Categorical snow'\n");
		  exit(1);
		case 37:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Convective precipitation rate'\n");
		  exit(1);
		case 38:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Horizontal moisture divergence'\n");
		  exit(1);
		case 39:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Percent frozen precipitation'\n");
		  exit(1);
		case 40:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Potential evaporation'\n");
		  exit(1);
		case 41:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Potential evaporation rate'\n");
		  exit(1);
		case 42:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Snow cover'\n");
		  exit(1);
		case 43:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Rain fraction of total water'\n");
		  exit(1);
		case 44:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Rime factor'\n");
		  exit(1);
		case 45:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Total column integrated rain'\n");
		  exit(1);
		case 46:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Total column integrated snow'\n");
		  exit(1);
		case 192:
		  switch (center) {
		    case 7:
			return 140;
		  }
		case 193:
		  switch (center) {
		    case 7:
			return 141;
		  }
		case 194:
		  switch (center) {
		    case 7:
			return 142;
		  }
		case 195:
		  switch (center) {
		    case 7:
			return 143;
		  }
		case 196:
		  switch (center) {
		    case 7:
			return 214;
		  }
		case 197:
		  switch (center) {
		    case 7:
			return 135;
		  }
		case 199:
		  switch (center) {
		    case 7:
			return 228;
		  }
		case 200:
		  switch (center) {
		    case 7:
			return 145;
		  }
		case 201:
		  switch (center) {
		    case 7:
			return 238;
		  }
		case 206:
		  switch (center) {
		    case 7:
			return 186;
		  }
		case 207:
		  switch (center) {
		    case 7:
			return 198;
		  }
		case 208:
		  switch (center) {
		    case 7:
			return 239;
		  }
		case 213:
		  switch (center) {
		    case 7:
			return 243;
		  }
		case 214:
		  switch (center) {
		    case 7:
			return 245;
		  }
		case 215:
		  switch (center) {
		    case 7:
			return 249;
		  }
		case 216:
		  switch (center) {
		    case 7:
			return 159;
		  }
	    }
	    break;
// momentum parameters
	  case 2:
	    switch(param_num) {
		case 0:
		  return 31;
		case 1:
		  return 32;
		case 2:
		  return 33;
		case 3:
		  return 34;
		case 4:
		  return 35;
		case 5:
		  return 36;
		case 6:
		  return 37;
		case 7:
		  return 38;
		case 8:
		  return 39;
		case 9:
		  return 40;
		case 10:
		  return 41;
		case 11:
		  return 42;
		case 12:
		  return 43;
		case 13:
		  return 44;
		case 14:
		  return 4;
		case 15:
		  return 45;
		case 16:
		  return 46;
		case 17:
		  return 124;
		case 18:
		  return 125;
		case 19:
		  return 126;
		case 20:
		  return 123;
		case 21:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Maximum wind speed'\n");
		  exit(1);
		case 22:
		  switch (center) {
		    case 7:
			return 180;
		    default:
			fprintf(stderr,"There is no GRIB1 parameter code for 'Wind speed (gust)'\n");
			exit(1);
		  }
		case 23:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'u-component of wind (gust)'\n");
		  exit(1);
		case 24:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'v-component of wind (gust)'\n");
		  exit(1);
		case 25:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Vertical speed shear'\n");
		  exit(1);
		case 26:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Horizontal momentum flux'\n");
		  exit(1);
		case 27:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'u-component storm motion'\n");
		  exit(1);
		case 28:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'v-component storm motion'\n");
		  exit(1);
		case 29:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Drag coefficient'\n");
		  exit(1);
		case 30:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Frictional velocity'\n");
		  exit(1);
		case 192:
		  switch (center) {
		    case 7:
			return 136;
		  }
		case 193:
		  switch (center) {
		    case 7:
			return 172;
		  }
		case 194:
		  switch (center) {
		    case 7:
			return 196;
		  }
		case 195:
		  switch (center) {
		    case 7:
			return 197;
		  }
		case 196:
		  switch (center) {
		    case 7:
			return 252;
		  }
		case 197:
		  switch (center) {
		    case 7:
			return 253;
		  }
	    }
	    break;
// mass parameters
	  case 3:
	    switch (param_num) {
		case 0:
		  return 1;
		case 1:
		  return 2;
		case 2:
		  return 3;
		case 3:
		  return 5;
		case 4:
		  return 6;
		case 5:
		  return 7;
		case 6:
		  return 8;
		case 7:
		  return 9;
		case 8:
		  return 26;
		case 9:
		  return 27;
		case 10:
		  return 89;
		case 11:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Altimeter setting'\n");
		  exit(1);
		case 12:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Thickness'\n");
		  exit(1);
		case 13:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Pressure altitude'\n");
		  exit(1);
		case 14:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Density altitude'\n");
		  exit(1);
		case 15:
		  fprintf(stderr,"There is no GRIB1 parameter code for '5-wave geopotential height'\n");
		  exit(1);
		case 16:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Zonal flux of gravity wave stress'\n");
		  exit(1);
		case 17:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Meridional flux of gravity wave stress'\n");
		  exit(1);
		case 18:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Planetary boundary layer height'\n");
		  exit(1);
		case 19:
		  fprintf(stderr,"There is no GRIB1 parameter code for '5-wave geopotential height anomaly'\n");
		  exit(1);
		case 192:
		  switch (center) {
		    case 7:
			return 130;
		  }
		case 193:
		  switch (center) {
		    case 7:
			return 222;
		  }
		case 194:
		  switch (center) {
		    case 7:
			return 147;
		  }
		case 195:
		  switch (center) {
		    case 7:
			return 148;
		  }
		case 196:
		  switch (center) {
		    case 7:
			return 221;
		  }
		case 197:
		  switch (center) {
		    case 7:
			return 230;
		  }
		case 198:
		  switch (center) {
		    case 7:
			return 129;
		  }
		case 199:
		  switch (center) {
		    case 7:
			return 137;
		  }
	    }
	    break;
// short-wave radiation parameters
	  case 4:
	    switch (param_num) {
		case 0:
		  return 111;
		case 1:
		  return 113;
		case 2:
		  return 116;
		case 3:
		  return 117;
		case 4:
		  return 118;
		case 5:
		  return 119;
		case 6:
		  return 120;
		case 7:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Downward short-wave radiation flux'\n");
		  exit(1);
		case 8:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Upward short-wave radiation flux'\n");
		  exit(1);
		case 192:
		  switch (center) {
		    case 7:
			return 204;
		  }
		case 193:
		  switch (center) {
		    case 7:
			return 211;
		  }
		case 196:
		  switch (center) {
		    case 7:
			return 161;
		  }
	    }
	    break;
// long-wave radiation parameters
	  case 5:
	    switch (param_num) {
		case 0:
		  return 112;
		case 1:
		  return 114;
		case 2:
		  return 115;
		case 3:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Downward long-wave radiation flux'\n");
		  exit(1);
		case 4:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Upward long-wave radiation flux'\n");
		  exit(1);
		case 192:
		  switch (center) {
		    case 7:
			return 205;
		  }
		case 193:
		  switch (center) {
		    case 7:
			return 212;
		  }
	    }
	    break;
// cloud parameters
	  case 6:
	    switch (param_num) {
		case 0:
		  return 58;
		case 1:
		  return 71;
		case 2:
		  return 72;
		case 3:
		  return 73;
		case 4:
		  return 74;
		case 5:
		  return 75;
		case 6:
		  return 76;
		case 7:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Cloud amount'\n");
		  exit(1);
		case 8:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Cloud type'\n");
		  exit(1);
		case 9:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Thunderstorm maximum tops'\n");
		  exit(1);
		case 10:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Thunderstorm coverage'\n");
		  exit(1);
		case 11:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Cloud base'\n");
		  exit(1);
		case 12:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Cloud top'\n");
		  exit(1);
		case 13:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Ceiling'\n");
		  exit(1);
		case 14:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Non-convective cloud cover'\n");
		  exit(1);
		case 15:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Cloud work function'\n");
		  exit(1);
		case 16:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Convective cloud efficiency'\n");
		  exit(1);
		case 17:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Total condensate'\n");
		  exit(1);
		case 18:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Total column-integrated cloud water'\n");
		  exit(1);
		case 19:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Total column-integrated cloud ice'\n");
		  exit(1);
		case 20:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Total column-integrated cloud condensate'\n");
		  exit(1);
		case 21:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Ice fraction of total condensate'\n");
		  exit(1);
		case 192:
		  switch (center) {
		    case 7:
			return 213;
		  }
		case 193:
		  switch (center) {
		    case 7:
			return 146;
		  }
	    }
	    break;
// thermodynamic stability index parameters
	  case 7:
	    switch (param_num) {
		case 0:
		  return 24;
		case 1:
		  return 77;
		case 2:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'K index'\n");
		  exit(1);
		case 3:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'KO index'\n");
		  exit(1);
		case 4:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Total totals index'\n");
		  exit(1);
		case 5:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Sweat index'\n");
		  exit(1);
		case 6:
		  switch (center) {
		    case 7:
			return 157;
		    default:
			fprintf(stderr,"There is no GRIB1 parameter code for 'Convective available potential energy'\n");
			exit(1);
		  }
		case 7:
		  switch (center) {
		    case 7:
			return 156;
		    default:
			fprintf(stderr,"There is no GRIB1 parameter code for 'Convective inhibition'\n");
			exit(1);
		  }
		case 8:
		  switch (center) {
		    case 7:
			return 190;
		    default:
			fprintf(stderr,"There is no GRIB1 parameter code for 'Storm-relative helicity'\n");
			exit(1);
		  }
		case 9:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Energy helicity index'\n");
		  exit(1);
		case 10:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Surface lifted index'\n");
		  exit(1);
		case 11:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Best (4-layer) lifted index'\n");
		  exit(1);
		case 12:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Richardson number'\n");
		  exit(1);
		case 192:
		  switch (center) {
		    case 7:
			return 131;
		  }
		case 193:
		  switch (center) {
		    case 7:
			return 132;
		  }
		case 194:
		  switch (center) {
		    case 7:
			return 254;
		  }
	    }
	    break;
// aerosol parameters
	  case 13:
	    switch (param_num) {
		case 0:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Aerosol type'\n");
		  exit(1);
	    }
	    break;
// trace gas parameters
	  case 14:
	    switch (param_num) {
		case 0:
		  return 10;
		case 1:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Ozone mixing ratio'\n");
		  exit(1);
		case 192:
		  switch (center) {
		    case 7:
			return 154;
		  }
	    }
	    break;
// radar parameters
	  case 15:
	    switch (param_num) {
		case 0:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Base spectrum width'\n");
		  exit(1);
		case 1:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Base reflectivity'\n");
		  exit(1);
		case 2:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Base radial velocity'\n");
		  exit(1);
		case 3:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Vertically-integrated liquid'\n");
		  exit(1);
		case 4:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Layer-maximum base reflectivity'\n");
		  exit(1);
		case 5:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Radar precipitation'\n");
		  exit(1);
		case 6:
		  return 21;
		case 7:
		  return 22;
		case 8:
		  return 23;
	    }
	    break;
// nuclear/radiology parameters
	  case 18:
	    break;
// physical atmospheric property parameters
	  case 19:
	    switch (param_num) {
		case 0:
		  return 20;
		case 1:
		  return 84;
		case 2:
		  return 60;
		case 3:
		  return 67;
		case 4:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Volcanic ash'\n");
		  exit(1);
		case 5:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Icing top'\n");
		  exit(1);
		case 6:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Icing base'\n");
		  exit(1);
		case 7:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Icing'\n");
		  exit(1);
		case 8:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Turbulence top'\n");
		  exit(1);
		case 9:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Turbulence base'\n");
		  exit(1);
		case 10:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Turbulence'\n");
		  exit(1);
		case 11:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Turbulent kinetic energy'\n");
		  exit(1);
		case 12:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Planetary boundary layer regime'\n");
		  exit(1);
		case 13:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Contrail intensity'\n");
		  exit(1);
		case 14:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Contrail engine type'\n");
		  exit(1);
		case 15:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Contrail top'\n");
		  exit(1);
		case 16:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Contrail base'\n");
		  exit(1);
		case 17:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Maximum snow albedo'\n");
		  exit(1);
		case 18:
		  fprintf(stderr,"There is no GRIB1 parameter code for 'Snow-free albedo'\n");
		  exit(1);
		case 204:
		  switch (center) {
		    case 7:
			return 209;
		  }
	    }
	    break;
	}
	break;
// hydrologic products
    case 1:
	switch (param_cat) {
// hydrology basic products
	  case 0:
	    switch (param_num) {
		case 192:
		  switch (center) {
		    case 7:
			return 234;
		  }
		case 193:
		  switch (center) {
		    case 7:
			return 235;
		  }
	    }
	  case 1:
	    switch (param_num) {
		case 192:
		  switch (center) {
		    case 7:
			return 194;
		  }
		case 193:
		  switch (center) {
		    case 7:
			return 195;
		  }
	    }
	    break;
	}
// land surface products
    case 2:
	switch (param_cat) {
// vegetation/biomass
	  case 0:
	    switch (param_num) {
		case 0:
		  return 81;
		case 1:
		  return 83;
		case 2:
		  return 85;
		case 3:
		  return 86;
		case 4:
		  return 87;
		case 5:
		  return 90;
		case 192:
		  switch (center) {
		    case 7:
			return 144;
		  }
		case 193:
		  switch (center) {
		    case 7:
			return 155;
		  }
		case 194:
		  switch (center) {
		    case 7:
			return 207;
		  }
		case 195:
		  switch (center) {
		    case 7:
			return 208;
		  }
		case 196:
		  switch (center) {
		    case 7:
			return 223;
		  }
		case 197:
		  switch (center) {
		    case 7:
			return 226;
		  }
		case 198:
		  switch (center) {
		    case 7:
			return 225;
		  }
		case 207:
		  switch (center) {
		    case 7:
			return 201;
		  }
	    }
	    break;
	}
	break;
// oceanographic products
    case 10:
	switch (param_cat) {
// waves parameters
	  case 0:
	    switch (param_num) {
		case 0:
		  return 28;
		case 1:
		  return 29;
		case 2:
		  return 30;
		case 3:
		  return 100;
		case 4:
		  return 101;
		case 5:
		  return 102;
		case 6:
		  return 103;
		case 7:
		  return 104;
		case 8:
		  return 105;
		case 9:
		  return 106;
		case 10:
		  return 107;
		case 11:
		  return 108;
		case 12:
		  return 109;
		case 13:
		  return 110;
	    }
	    break;
// currents parameters
	  case 1:
	    switch (param_num) {
		case 0:
		  return 47;
		case 1:
		  return 48;
		case 2:
		  return 49;
		case 3:
		  return 50;
	    }
	    break;
// ice parameters
	  case 2:
	    switch (param_num) {
		case 0:
		  return 91;
		case 1:
		  return 92;
		case 2:
		  return 93;
		case 3:
		  return 94;
		case 4:
		  return 95;
		case 5:
		  return 96;
		case 6:
		  return 97;
		case 7:
		  return 98;
	    }
	    break;
// surface properties parameters
	  case 3:
	    switch (param_num) {
		case 0:
		  return 80;
		case 1:
		  return 82;
	    }
	    break;
// sub-surface properties parameters
	  case 4:
	    switch (param_num) {
		case 0:
		  return 69;
		case 1:
		  return 70;
		case 2:
		  return 68;
		case 3:
		  return 88;
	    }
	    break;
	}
	break;
  }
  fprintf(stderr,"There is no GRIB1 parameter code for discipline %d, parameter category %d, parameter number %d\n",disc,param_cat,param_num);
  exit(1);
}

int mapLevelData(GRIB2Grid *grid,int *level_type,int *level1,int *level2,int center)
{
  if (grid->md.lvl2_type != 255 && grid->md.lvl1_type != grid->md.lvl2_type) {
    fprintf(stderr,"Unable to indicate a layer bounded by different level types %d and %d in GRIB1\n",grid->md.lvl1_type,grid->md.lvl2_type);
    exit(1);
  }
  *level1=*level2=0;
  switch (grid->md.lvl1_type) {
    case 1:
	*level_type=1;
	break;
    case 2:
	*level_type=2;
	break;
    case 3:
	*level_type=3;
	break;
    case 4:
	*level_type=4;
	break;
    case 5:
	*level_type=5;
	break;
    case 6:
	*level_type=6;
	break;
    case 7:
	*level_type=7;
	break;
    case 8:
	*level_type=8;
	break;
    case 9:
	*level_type=9;
	break;
    case 20:
	*level_type=20;
	break;
    case 100:
	if (grid->md.lvl2_type == 255) {
	  *level_type=100;
	  *level1=grid->md.lvl1/100.;
	}
	else {
	  *level_type=101;
	  *level1=grid->md.lvl1/1000.;
	  *level2=grid->md.lvl2/1000.;
	}
	break;
    case 101:
	*level_type=102;
	break;
    case 102:
	if (grid->md.lvl2_type == 255) {
	  *level_type=103;
	  *level1=grid->md.lvl1;
	}
	else {
	  *level_type=104;
	  *level1=grid->md.lvl1/100.;
	  *level2=grid->md.lvl2/100.;
	}
	break;
    case 103:
	if (grid->md.lvl2_type == 255) {
	  *level_type=105;
	  *level1=grid->md.lvl1;
	}
	else {
	  *level_type=106;
	  *level1=grid->md.lvl1/100.;
	  *level2=grid->md.lvl2/100.;
	}
	break;
    case 104:
	if (grid->md.lvl2_type == 255) {
	  *level_type=107;
	  *level1=grid->md.lvl1*10000.;
	}
	else {
	  *level_type=108;
	  *level1=grid->md.lvl1*100.;
	  *level2=grid->md.lvl2*100.;
	}
	break;
    case 105:
	*level1=grid->md.lvl1;
	if (grid->md.lvl2_type == 255)
	  *level_type=109;
	else {
	  *level_type=110;
	  *level2=grid->md.lvl2;
	}
	break;
    case 106:
	*level1=grid->md.lvl1*100.;
	if (grid->md.lvl2_type == 255)
	  *level_type=111;
	else {
	  *level_type=112;
	  *level2=grid->md.lvl2*100.;
	}
	break;
    case 107:
	if (grid->md.lvl2_type == 255) {
	  *level_type=113;
	  *level1=grid->md.lvl1;
	}
	else {
	  *level_type=114;
	  *level1=475.-grid->md.lvl1;
	  *level2=475.-grid->md.lvl2;
	}
	break;
    case 108:
	*level1=grid->md.lvl1/100.;
	if (grid->md.lvl2_type == 255)
	  *level_type=115;
	else {
	  *level_type=116;
	  *level2=grid->md.lvl2/100.;
	}
	break;
    case 109:
	*level_type=117;
	*level1=grid->md.lvl1*1000000000.;
	break;
    case 111:
	if (grid->md.lvl2_type == 255) {
	  *level_type=119;
	  *level1=grid->md.lvl1*10000.;
	}
	else {
	  *level_type=120;
	  *level1=grid->md.lvl1*100.;
	  *level2=grid->md.lvl2*100.;
	}
	break;
    case 117:
	fprintf(stderr,"There is no GRIB1 level code for 'Mixed layer depth'\n");
	exit(1);
    case 160:
	*level_type=160;
	*level1=grid->md.lvl1;
	break;
    case 200:
	switch (center) {
	  case 7:
	    *level_type=200;
	    break;
	}
	break;
  }
}

int mapStatisticalEndTime(GRIBMessage *msg,GRIB2Grid *grid)
{
  switch (grid->md.time_unit) {
    case 0:
	return (grid->md.stat_proc.etime/100 % 100)-(msg->time/100 % 100);
    case 1:
	 return (grid->md.stat_proc.etime/10000-msg->time/10000);
    case 2:
	return (grid->md.stat_proc.edy-msg->dy);
    case 3:
	return (grid->md.stat_proc.emo-msg->mo);
    case 4:
	return (grid->md.stat_proc.eyr-msg->yr);
    default:
	fprintf(stderr,"Unable to map end time with units %d to GRIB1\n",grid->md.time_unit);
	exit(1);
  }
}

void mapTimeRange(GRIBMessage *msg,GRIB2Grid *grid,int *p1,int *p2,int *t_range,int *n_avg,int *n_missing,int center)
{
  size_t n;

  switch (grid->md.pds_templ_num) {
    case 0:
    case 1:
    case 2:
	*t_range=0;
	*p1=grid->md.fcst_time;
	*p2=0;
	*n_avg=*n_missing=0;
	break;
    case 8:
    case 11:
    case 12:
	if (grid->md.stat_proc.num_ranges > 1) {
	  if (center == 7 && grid->md.stat_proc.num_ranges == 2) {
/* NCEP CFSR monthly grids */
	    *p2=grid->md.stat_proc.incr_length[0];
	    *p1=*p2-grid->md.stat_proc.time_length[1];
	    *n_avg=grid->md.stat_proc.time_length[0];
	    switch (grid->md.stat_proc.proc_code[0]) {
		case 193:
		  *t_range=113;
		  break;
		case 194:
		  *t_range=123;
		  break;
		case 195:
		  *t_range=128;
		  break;
		case 196:
		  *t_range=129;
		  break;
		case 197:
		  *t_range=130;
		  break;
		case 198:
		  *t_range=131;
		  break;
		case 199:
		  *t_range=132;
		  break;
		case 200:
		  *t_range=133;
		  break;
		case 201:
		  *t_range=134;
		  break;
		case 202:
		  *t_range=135;
		  break;
		case 203:
		  *t_range=136;
		  break;
		case 204:
		  *t_range=137;
		  break;
		case 205:
		  *t_range=138;
		  break;
		case 206:
		  *t_range=139;
		  break;
		case 207:
		  *t_range=140;
		  break;
		default:
		  fprintf(stderr,"Unable to map NCEP statistical process code %d to GRIB1\n",grid->md.stat_proc.proc_code[0]);
		  exit(1);
	    }
	  }
	  else {
	    fprintf(stderr,"Unable to map multiple statistical processes to GRIB1\n");
	    exit(1);
	  }
	}
	else {
	  switch (grid->md.stat_proc.proc_code[0]) {
	    case 0:
	    case 1:
	    case 4:
		switch (grid->md.stat_proc.proc_code[0]) {
/* average */
		  case 0:
		    *t_range=3;
		    break;
/* accumulation */
		  case 1:
		    *t_range=4;
		    break;
/* difference */
		  case 4:
		    *t_range=5;
		    break;
		}
		*p1=grid->md.fcst_time;
		*p2=mapStatisticalEndTime(msg,grid);
		if (grid->md.stat_proc.incr_length[0] == 0)
		  *n_avg=0;
		else {
		  fprintf(stderr,"Unable to map discrete processing to GRIB1\n");
		  exit(1);
		}
		break;
// maximum
	    case 2:
// minimum
	    case 3:
		*t_range=2;
		*p1=grid->md.fcst_time;
		*p2=mapStatisticalEndTime(msg,grid);
		if (grid->md.stat_proc.incr_length[0] == 0)
		  *n_avg=0;
		else {
		  fprintf(stderr,"Unable to map discrete processing to GRIB1\n");
		  exit(1);
		}
		break;
	    default:
// patch for NCEP grids
		if (grid->md.stat_proc.proc_code[0] == 255 && center == 7) {
 		  if (msg->disc == 0) {
		    if (grid->md.param_cat == 0) {
			switch (grid->md.param_num) {
			  case 4:
			  case 5:
			    *t_range=2;
			    *p1=grid->md.fcst_time;
			    *p2=mapStatisticalEndTime(msg,grid);
			    if (grid->md.stat_proc.incr_length[0] == 0)
				*n_avg=0;
			    else {
				fprintf(stderr,"Unable to map discrete processing to GRIB1\n");
				exit(1);
			    }
			    break;
			}
		    }
		  }
		}
		else {
		  fprintf(stderr,"Unable to map statistical process %d to GRIB1\n",grid->md.stat_proc.proc_code[0]);
		  exit(1);
		}
	  }
	}
	*n_missing=grid->md.stat_proc.nmiss;
	break;
    default:
	fprintf(stderr,"Unable to map time range for Product Definition Template %d into GRIB1\n",grid->md.pds_templ_num);
	exit(1);
  }
}

void packPDS(GRIBMessage *msg,int grid_number,unsigned char *grib1_buffer,size_t *offset)
{
  int level_type,level1,level2,p1,p2,t_range,n_avg,n_missing,D;
  static short warned_ensemble=0;

// length of the PDS
  setBits(grib1_buffer,28,*offset,24);
// GRIB1 tables version number
  setBits(grib1_buffer,3,*offset+24,8);
// originating center ID
  setBits(grib1_buffer,msg->center_id,*offset+32,8);
// generating process ID
  setBits(grib1_buffer,msg->grids[grid_number].md.gen_proc,*offset+40,8);
// grid definition catalog number - set to 255 because GDS is to be included
  setBits(grib1_buffer,255,*offset+48,8);
// flag
  if (msg->grids[grid_number].md.bitmap == NULL)
    setBits(grib1_buffer,0x80,*offset+56,8);
  else
    setBits(grib1_buffer,0xc0,*offset+56,8);
// parameter code
  setBits(grib1_buffer,mapParameterData(msg->center_id,msg->disc,msg->grids[grid_number].md.param_cat,msg->grids[grid_number].md.param_num),*offset+64,8);
  mapLevelData(&msg->grids[grid_number],&level_type,&level1,&level2,msg->center_id);
// level type code
  setBits(grib1_buffer,level_type,*offset+72,8);
  if (msg->grids[grid_number].md.lvl2_type == 255)
    setBits(grib1_buffer,level1,*offset+80,16);
  else {
    setBits(grib1_buffer,level1,*offset+80,8);
    setBits(grib1_buffer,level2,*offset+88,8);
  }
// year of century
  setBits(grib1_buffer,(msg->yr % 100),*offset+96,8);
// month
  setBits(grib1_buffer,msg->mo,*offset+104,8);
// day
  setBits(grib1_buffer,msg->dy,*offset+112,8);
// hour
  setBits(grib1_buffer,msg->time/10000,*offset+120,8);
// minute
  setBits(grib1_buffer,(msg->time/100 % 100),*offset+128,8);
// second
  if (msg->md.time_unit == 13)
    fprintf(stderr,"Unable to indicate 'Second' for time unit in GRIB1\n");
  else
    setBits(grib1_buffer,msg->md.time_unit,*offset+136,8);
  mapTimeRange(msg,&msg->grids[grid_number],&p1,&p2,&t_range,&n_avg,&n_missing,msg->center_id);
  if (t_range == 10)
    setBits(grib1_buffer,p1,*offset+144,16);
  else {
    setBits(grib1_buffer,p1,*offset+144,8);
    setBits(grib1_buffer,p2,*offset+152,8);
  }
  setBits(grib1_buffer,t_range,*offset+160,8);
// century of year
  setBits(grib1_buffer,(msg->yr/100)+1,*offset+192,8);
// originating sub-center ID
  setBits(grib1_buffer,msg->sub_center_id,*offset+200,8);
// decimal scale factor
  D=msg->md.D;
  if (D < 0)
    D=-D+0x8000;
  setBits(grib1_buffer,D,*offset+208,16);
  (*offset)+=224;
  if (msg->md.ens_type >= 0) {
// length of the PDS
    setBits(grib1_buffer,43,*offset-224,24);
    setBits(grib1_buffer,msg->md.ens_type,*offset+96,8);
    setBits(grib1_buffer,msg->md.perturb_num,*offset+104,8);
    setBits(grib1_buffer,msg->md.nfcst_in_ensemble,*offset+112,8);
    (*offset)+=120;
    if (warned_ensemble == 0) {
	fprintf(stderr,"Notice: the 'Ensemble type code', the 'Perturbation Number', and the\n");
	fprintf(stderr,"'Number of forecasts in ensemble' from Product Definition Template 4.1 and/or\n");
	fprintf(stderr,"Product Definition Template 4.12 have been packed in octets 41, 42, and 43 of\n");
	fprintf(stderr,"the GRIB1 Product Definition Section\n");
	warned_ensemble=1;
    }
  }
  else if (msg->md.derived_fcst_code >= 0) {
// length of the PDS
    setBits(grib1_buffer,42,*offset-224,24);
    setBits(grib1_buffer,msg->md.derived_fcst_code,*offset+96,8);
    setBits(grib1_buffer,msg->md.nfcst_in_ensemble,*offset+104,8);
    (*offset)+=112;
    if (warned_ensemble == 0) {
	fprintf(stderr,"Notice: the 'Derived forecast code' and the 'Number of forecasts in ensemble'\n");
	fprintf(stderr,"from Product Definition Template 4.2 and/or Product Definition Template 4.12\n");
	fprintf(stderr,"have been packed in octets 41 and 42 of the GRIB1 Product Definition Section\n");
	warned_ensemble=1;
    }
  }
}

void packGDS(GRIBMessage *msg,int grid_number,unsigned char *grib1_buffer,size_t *offset)
{
  int rescomp=0,sign,value;

// NV
  setBits(grib1_buffer,255,*offset+24,8);
// PV
  setBits(grib1_buffer,255,*offset+32,8);
  switch (msg->md.gds_templ_num) {
    case 0:
// length of the GDS
	setBits(grib1_buffer,32,*offset,24);
// data representation
	setBits(grib1_buffer,0,*offset+40,8);
// Ni
	setBits(grib1_buffer,msg->md.nx,*offset+48,16);
// Nj
	setBits(grib1_buffer,msg->md.ny,*offset+64,16);
// first latitude
	value=msg->md.slat*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+80,1);
	  setBits(grib1_buffer,value,*offset+81,23);
	}
	else
	  setBits(grib1_buffer,value,*offset+80,24);
// first longitude
	value=msg->md.slon*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+104,1);
	  setBits(grib1_buffer,value,*offset+105,23);
	}
	else
	  setBits(grib1_buffer,value,*offset+104,24);
// resolution and component flags
	if (msg->md.rescomp&0x20 == 0x20)
	  rescomp|=0x80;
	if (msg->md.earth_shape == 2)
	  rescomp|=0x40;
	if (msg->md.rescomp&0x8 == 0x8)
	  rescomp|=0x8;
	setBits(grib1_buffer,rescomp,*offset+128,8);
// last latitude
	value=msg->md.lats.elat*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+136,1);
	  setBits(grib1_buffer,value,*offset+137,23);
	}
	else
	  setBits(grib1_buffer,value,*offset+136,24);
// last longitude
	value=msg->md.lons.elon*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+160,1);
	  setBits(grib1_buffer,value,*offset+161,23);
	}
	else
	  setBits(grib1_buffer,value,*offset+160,24);
// Di increment
	value=msg->md.xinc.loinc*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+184,1);
	  setBits(grib1_buffer,value,*offset+185,15);
	}
	else
	  setBits(grib1_buffer,value,*offset+184,16);
// Dj increment
	value=msg->md.yinc.lainc*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+200,1);
	  setBits(grib1_buffer,value,*offset+201,15);
	}
	else
	  setBits(grib1_buffer,value,*offset+200,16);
// scanning mode
	setBits(grib1_buffer,msg->md.scan_mode,*offset+216,8);
// reserved
	setBits(grib1_buffer,0,*offset+224,32);
	(*offset)+=256;
	break;
    case 30:
// length of the GDS
	setBits(grib1_buffer,42,*offset,24);
// data representation
	setBits(grib1_buffer,3,*offset+40,8);
// Nx
	setBits(grib1_buffer,msg->md.nx,*offset+48,16);
// Ny
	setBits(grib1_buffer,msg->md.ny,*offset+64,16);
// first latitude
	value=msg->md.slat*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+80,1);
	  setBits(grib1_buffer,value,*offset+81,23);
	}
	else
	  setBits(grib1_buffer,value,*offset+80,24);
// first longitude
	value=msg->md.slon*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+104,1);
	  setBits(grib1_buffer,value,*offset+105,23);
	}
	else
	  setBits(grib1_buffer,value,*offset+104,24);
// resolution and component flags
	if (msg->md.rescomp&0x20 == 0x20)
	  rescomp|=0x80;
	if (msg->md.earth_shape == 2)
	  rescomp|=0x40;
	if (msg->md.rescomp&0x8 == 0x8)
	  rescomp|=0x8;
	setBits(grib1_buffer,rescomp,*offset+128,8);
// LoV
	value=msg->md.lons.lov*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+136,1);
	  setBits(grib1_buffer,value,*offset+137,23);
	}
	else
	  setBits(grib1_buffer,value,*offset+136,24);
// Dx
	value=msg->md.xinc.dxinc+0.5;
	setBits(grib1_buffer,value,*offset+160,24);
// Dy
	value=msg->md.yinc.dyinc+0.5;
	setBits(grib1_buffer,value,*offset+184,24);
// projection center flag
	setBits(grib1_buffer,msg->md.proj_flag,*offset+208,8);
// scanning mode
	setBits(grib1_buffer,msg->md.scan_mode,*offset+216,8);
// latin1
	value=msg->md.latin1*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+224,1);
	  setBits(grib1_buffer,value,*offset+225,23);
	}
	else
	  setBits(grib1_buffer,value,*offset+224,24);
// latin2
	value=msg->md.latin2*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+248,1);
	  setBits(grib1_buffer,value,*offset+249,23);
	}
	else
	  setBits(grib1_buffer,value,*offset+248,24);
// latitude of southern pole of projection
	value=msg->md.splat*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+272,1);
	  setBits(grib1_buffer,value,*offset+273,23);
	}
	else
	  setBits(grib1_buffer,value,*offset+272,24);
// longitude of southern pole of projection
	value=msg->md.splon*1000.;
	if (value < 0.) {
	  value=-value;
	  setBits(grib1_buffer,1,*offset+296,1);
	  setBits(grib1_buffer,value,*offset+297,23);
	}
	else
	  setBits(grib1_buffer,value,*offset+296,24);
// reserved
	setBits(grib1_buffer,0,*offset+320,16);
	(*offset)+=336;
	break;
    default:
	fprintf(stderr,"Unable to map Grid Definition Template %d into GRIB1\n",msg->md.gds_templ_num);
	exit(1);
  }
}

void packBMS(GRIBMessage *msg,int grid_number,unsigned char *grib1_buffer,size_t *offset,int num_points)
{
  int length=6+(num_points+7)/8;
  int ub=8-(num_points % 8);
  size_t n,off;

// length of the BMS
  setBits(grib1_buffer,length,*offset,24);
// unused bits at end of section
  setBits(grib1_buffer,ub,*offset+24,8);
// table reference
  setBits(grib1_buffer,0,*offset+32,16);
// the bitmap
  off=*offset+48;
  for (n=0; n < num_points; n++)
    setBits(grib1_buffer,msg->grids[grid_number].md.bitmap[n],off++,1);
  (*offset)+=length*8;
}

int ieee2ibm(double ieee)
{
  int ibm_real=0;
  unsigned char *ir=(unsigned char *)&ibm_real;
  int sign=0,fr=0;
  int exp=64;
  const double full=0xffffff;
  size_t size=sizeof(size_t)*8,off=0;

  if (ieee != 0.) {
    if (ieee < 0.) {
      sign=1;
      ieee=-ieee;
    }
    ieee/=pow(2.,-24.);
    while (exp > 0 && ieee < full) {
      ieee*=16.;
      exp--;
    }
    while (ieee > full) {
      ieee/=16.;
      exp++;
    }
    fr=ieee+0.5;
    if (size > 32) {
      off=size-32;
      setBits(ir,0,0,off);
    }
    setBits(ir,sign,off,1);
    setBits(ir,exp,off+1,7);
    setBits(ir,fr,off+8,24);
  }
  return ibm_real;
}

void packBDS(GRIBMessage *msg,int grid_number,unsigned char *grib1_buffer,size_t *offset,int *pvals,int num_to_pack,int pack_width)
{
  int length=11+(num_to_pack*pack_width+7)/8;
  size_t m,off;
  int E,ibm_rep;

// length of the BDS
  setBits(grib1_buffer,length,*offset,24);
// flag
  setBits(grib1_buffer,0,*offset+24,4);
// unused bits
  setBits(grib1_buffer,(length-11)*8-(num_to_pack*pack_width),*offset+28,4);
// scale factor E
  E=msg->grids[grid_number].md.E;
  if (E < 0)
    E=-E+0x8000;
  setBits(grib1_buffer,E,*offset+32,16);
// Reference value
  ibm_rep=ieee2ibm(msg->grids[grid_number].md.R*pow(10.,msg->grids[grid_number].md.D));
  memcpy(&grib1_buffer[(*offset+48)/8],&ibm_rep,4);
// width in bits of each packed value
  setBits(grib1_buffer,pack_width,*offset+80,8);
// packed data values
  off=*offset+88;
  for (m=0; m < num_to_pack; m++) {
    setBits(grib1_buffer,pvals[m],off,pack_width);
    off+=pack_width;
  }
}

int main(int argc,char **argv)
{
  GRIBMessage grib_msg;
  FILE *fp,*ofp;
  size_t nmsg=0,ngrid=0;
  int status;
  int length,max_length=0,num_points,num_to_pack,pack_width,*pvals,max_pack;
  unsigned char *grib1_buffer=NULL,dum[3];
  char *head="GRIB",*tail="7777";
  size_t n,m,offset,cnt;

  grib_msg.buffer=NULL;
  grib_msg.grids=NULL;

  if (argc != 3) {
    fprintf(stderr,"usage: %s GRIB2_file_name GRIB1_file_name\n",argv[0]);
    exit(1);
  }
  fp=fopen(argv[1],"rb");
  ofp=fopen(argv[2],"wb");

  while ( (status=unpackgrib2(fp,&grib_msg)) == 0) {
    nmsg++;
    for (n=0; n < grib_msg.num_grids; n++) {
// calculate the octet length of the GRIB1 grid (minus the Indicator and End
// Sections, which are both fixed in length
	switch (grib_msg.md.pds_templ_num) {
	  case 0:
	  case 8:
	    length=28;
	    break;
	  case 1:
	  case 11:
	    length=43;
	    break;
	  case 2:
	  case 12:
	    length=42;
	    break;
	  default:
	    fprintf(stderr,"Unable to map Product Definition Template %d into GRIB1\n",grib_msg.md.pds_templ_num);
	    exit(1);
	}
	switch (grib_msg.md.gds_templ_num) {
	  case 0:
	    length+=32;
	    num_points=grib_msg.md.nx*grib_msg.md.ny;
	    break;
	  case 30:
	    length+=42;
	    num_points=grib_msg.md.nx*grib_msg.md.ny;
	    break;
	  default:
	    fprintf(stderr,"Unable to map Grid Definition Template %d into GRIB1\n",grib_msg.md.gds_templ_num);
	    exit(1);
	}
	if (grib_msg.grids[n].md.bitmap != NULL) {
	  length+=6+(num_points+7)/8;
	  num_to_pack=0;
	  for (m=0; m < num_points; m++) {
	    if (grib_msg.grids[n].md.bitmap[m] == 1)
		num_to_pack++;
	  }
	}
	else
	  num_to_pack=num_points;
	pvals=(int *)malloc(sizeof(int)*num_to_pack);
	max_pack=0;
	cnt=0;
	for (m=0; m < num_points; m++) {
	  if (grib_msg.grids[n].gridpoints[m] != GRIB_MISSING_VALUE) {
	    pvals[cnt]=lroundf((grib_msg.grids[n].gridpoints[m]-grib_msg.grids[n].md.R)*pow(10.,grib_msg.grids[n].md.D)/pow(2.,grib_msg.grids[n].md.E));
	    if (pvals[cnt] > max_pack)
		max_pack=pvals[cnt];
	    cnt++;
	  }
	}
	pack_width=1;
	while (pow(2.,pack_width)-1 < max_pack)
	  pack_width++;
	length+=11+(num_to_pack*pack_width+7)/8;
// allocate enough memory for the GRIB1 buffer
	if (length > max_length) {
	  if (grib1_buffer != NULL)
	    free(grib1_buffer);
	  grib1_buffer=(unsigned char *)malloc(length*sizeof(unsigned char));
	  max_length=length;
	}
	offset=0;
// pack the Product Definition Section
	packPDS(&grib_msg,n,grib1_buffer,&offset);
// pack the Grid Definition Section
	packGDS(&grib_msg,n,grib1_buffer,&offset);
// pack the Bitmap Section, if it exists
	if (grib_msg.grids[n].md.bitmap != NULL)
	  packBMS(&grib_msg,n,grib1_buffer,&offset,num_points);
// pack the Binary Data Section
	packBDS(&grib_msg,n,grib1_buffer,&offset,pvals,num_to_pack,pack_width);
	free(pvals);
// output the GRIB1 grid
	fwrite(head,1,4,ofp);
	setBits(dum,length+12,0,24);
	fwrite(dum,1,3,ofp);
	dum[0]=1;
	fwrite(dum,1,1,ofp);
	fwrite(grib1_buffer,1,length,ofp);
	fwrite(tail,1,4,ofp);
	ngrid++;
    }
  }
  if (status != -1)
    printf("Read error after %d messages\n",nmsg);
  printf("Number of GRIB1 grids written to output: %d\n",ngrid);
  fclose(fp);
  fclose(ofp);
}
