#include <grib2_conv.h>
#include <set_bits.h>
#include <conv_float.h>
#include <math.h>
#include <string.h>

#define UNUSED_ARG(a) (void)(a)

static int map_statistical_end_time(GRIBMessage * msg, GRIB2Grid * grid)
{
	switch (grid->md.time_unit) {
		case 0: return (grid->md.stat_proc.etime / 100 % 100) - (msg->time / 100 % 100);
		case 1: return grid->md.stat_proc.etime / 10000 - msg->time / 10000;
		case 2: return grid->md.stat_proc.edy - msg->dy;
		case 3: return grid->md.stat_proc.emo - msg->mo;
		case 4: return grid->md.stat_proc.eyr - msg->yr;
		default: break;
	}
	fprintf(stderr,"Unable to map end time with units %d to GRIB1\n", grid->md.time_unit);
	return -1;
}

static int map_parameter_meterological_temperature(int center, int disc, int param_cat, int param_num)
{
	const char * err = "<unknown>";
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	switch (param_num) {
		case  0: return 11;
		case  1: return 12;
		case  2: return 13;
		case  3: return 14;
		case  4: return 15;
		case  5: return 16;
		case  6: return 17;
		case  7: return 18;
		case  8: return 19;
		case  9: return 25;
		case 10: return 121;
		case 11: return 122;
		case 12: err = "Heat index"; break;
		case 13: err = "Wind chill factor"; break;
		case 14: err = "Minimum dew point depression"; break;
		case 15: err = "Virtual potential temperature"; break;
		case 16: err = "Snow phase change heat flux"; break;
		case 192:
			switch (center) {
				case 7: return 229;
				default: break;
			}
			break;
		default: break;
	}
	fprintf(stderr, "There is no GRIB1 parameter code for '%s' (%s:%d)\n", err, __FILE__, __LINE__);
	return -1;
}

static int map_parameter_meterological_moisture(int center, int disc, int param_cat, int param_num)
{
	const char * err = "<unknown>";
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	switch (param_num) {
		case  0: return 51;
		case  1: return 52;
		case  2: return 53;
		case  3: return 54;
		case  4: return 55;
		case  5: return 56;
		case  6: return 57;
		case  7: return 59;
		case  8: return 61;
		case  9: return 62;
		case 10: return 63;
		case 11: return 66;
		case 12: return 64;
		case 13: return 65;
		case 14: return 78;
		case 15: return 79;
		case 16: return 99;
		case 17: err = "Snow age"; break;
		case 18: err = "Absolute humidity"; break;
		case 19: err = "Precipitation type"; break;
		case 20: err = "Integrated liquid water"; break;
		case 21: err = "Condensate water"; break;
		case 22:
			switch (center) {
				case 7: return 153;
				default: err = "Cloud mixing ratio"; break;
			}
			break;
		case 23: err = "Ice water mixing ratio"; break;
		case 24: err = "Rain mixing ratio"; break;
		case 25: err = "Snow mixing ratio"; break;
		case 26: err = "Horizontal moisture convergence"; break;
		case 27: err = "Maximum relative humidity"; break;
		case 28: err = "Maximum absolute humidity"; break;
		case 29: err = "Total snowfall"; break;
		case 30: err = "Precipitable water category"; break;
		case 31: err = "Hail"; break;
		case 32: err = "Graupel (snow pellets)"; break;
		case 33: err = "Categorical rain"; break;
		case 34: err = "Categorical freezing rain"; break;
		case 35: err = "Categorical ice pellets"; break;
		case 36: err = "Categorical snow"; break;
		case 37: err = "Convective precipitation rate"; break;
		case 38: err = "Horizontal moisture divergence"; break;
		case 39: err = "Percent frozen precipitation"; break;
		case 40: err = "Potential evaporation"; break;
		case 41: err = "Potential evaporation rate"; break;
		case 42: err = "Snow cover"; break;
		case 43: err = "Rain fraction of total water"; break;
		case 44: err = "Rime factor"; break;
		case 45: err = "Total column integrated rain"; break;
		case 46: err = "Total column integrated snow"; break;
		case 192:
			switch (center) {
				case 7: return 140;
				default: break;
			}
			break;
		case 193:
			switch (center) {
				case 7: return 141;
				default: break;
			}
			break;
		case 194:
			switch (center) {
				case 7: return 142;
				default: break;
			}
			break;
		case 195:
			switch (center) {
				case 7: return 143;
				default: break;
			}
			break;
		case 196:
			switch (center) {
				case 7: return 214;
				default: break;
			}
			break;
		case 197:
			switch (center) {
				case 7: return 135;
				default: break;
			}
			break;
		case 199:
			switch (center) {
				case 7: return 228;
				default: break;
			}
			break;
		case 200:
			switch (center) {
				case 7: return 145;
				default: break;
			}
			break;
		case 201:
			switch (center) {
				case 7: return 238;
				default: break;
			}
			break;
		case 206:
			switch (center) {
				case 7: return 186;
				default: break;
			}
			break;
		case 207:
			switch (center) {
				case 7: return 198;
				default: break;
			}
			break;
		case 208:
			switch (center) {
				case 7: return 239;
				default: break;
			}
			break;
		case 213:
			switch (center) {
				case 7: return 243;
				default: break;
			}
			break;
		case 214:
			switch (center) {
				case 7: return 245;
				default: break;
			}
			break;
		case 215:
			switch (center) {
				case 7: return 249;
				default: break;
			}
			break;
		case 216:
			switch (center) {
				case 7: return 159;
				default: break;
			}
			break;
		default: break;
	}
	fprintf(stderr, "There is no GRIB1 parameter code for '%s' (%s:%d)\n", err, __FILE__, __LINE__);
	return -1;
}

static int map_parameter_meterological_momentum(int center, int disc, int param_cat, int param_num)
{
	const char * err = "<unknown>";
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	switch (param_num) {
		case  0: return 31;
		case  1: return 32;
		case  2: return 33;
		case  3: return 34;
		case  4: return 35;
		case  5: return 36;
		case  6: return 37;
		case  7: return 38;
		case  8: return 39;
		case  9: return 40;
		case 10: return 41;
		case 11: return 42;
		case 12: return 43;
		case 13: return 44;
		case 14: return 4;
		case 15: return 45;
		case 16: return 46;
		case 17: return 124;
		case 18: return 125;
		case 19: return 126;
		case 20: return 123;
		case 21: err = "Maximum wind speed"; break;
		case 22:
			switch (center) {
				case 7: return 180;
				default: err ="'Wind speed (gust)"; break;
			}
			break;
		case 23: err ="u-component of wind (gust)"; break;
		case 24: err ="v-component of wind (gust)"; break;
		case 25: err ="Vertical speed shear"; break;
		case 26: err ="Horizontal momentum flux"; break;
		case 27: err ="u-component storm motion"; break;
		case 28: err ="v-component storm motion"; break;
		case 29: err ="Drag coefficient"; break;
		case 30: err ="Frictional velocity"; break;
		case 192:
			switch (center) {
				case 7: return 136;
				default: break;
			}
			break;
		case 193:
			switch (center) {
				case 7: return 172;
				default: break;
			}
			break;
		case 194:
			switch (center) {
				case 7: return 196;
				default: break;
			}
			break;
		case 195:
			switch (center) {
				case 7: return 197;
				default: break;
			}
			break;
		case 196:
			switch (center) {
				case 7: return 252;
				default: break;
			}
			break;
		case 197:
			switch (center) {
				case 7: return 253;
				default: break;
			}
			break;
		default: break;
	}
	fprintf(stderr, "There is no GRIB1 parameter code for '%s' (%s:%d)\n", err, __FILE__, __LINE__);
	return -1;
}

static int map_parameter_meterological_mass(int center, int disc, int param_cat, int param_num)
{
	const char * err = "<unknown>";
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	switch (param_num) {
		case  0: return 1;
		case  1: return 2;
		case  2: return 3;
		case  3: return 5;
		case  4: return 6;
		case  5: return 7;
		case  6: return 8;
		case  7: return 9;
		case  8: return 26;
		case  9: return 27;
		case 10: return 89;
		case 11: err = "Altimeter setting"; break;
		case 12: err = "'Thickness"; break;
		case 13: err = "'Pressure altitude"; break;
		case 14: err = "'Density altitude"; break;
		case 15: err = "'5-wave geopotential height"; break;
		case 16: err = "'Zonal flux of gravity wave stress"; break;
		case 17: err = "'Meridional flux of gravity wave stress"; break;
		case 18: err = "'Planetary boundary layer height"; break;
		case 19: err = "'5-wave geopotential height anomaly"; break;
		case 192:
			switch (center) {
				case 7: return 130;
				default: break;
			}
			break;
		case 193:
			switch (center) {
				case 7: return 222;
				default: break;
			}
			break;
		case 194:
			switch (center) {
				case 7: return 147;
				default: break;
			}
			break;
		case 195:
			switch (center) {
				case 7: return 148;
				default: break;
			}
			break;
		case 196:
			switch (center) {
				case 7: return 221;
				default: break;
			}
			break;
		case 197:
			switch (center) {
				case 7: return 230;
				default: break;
			}
			break;
		case 198:
			switch (center) {
				case 7: return 129;
				default: break;
			}
			break;
		case 199:
			switch (center) {
				case 7: return 137;
				default: break;
			}
			break;
		default: break;
	}
	fprintf(stderr, "There is no GRIB1 parameter code for '%s' (%s:%d)\n", err, __FILE__, __LINE__);
	return -1;
}

static int map_parameter_meterological_shortwaveradiation(int center, int disc, int param_cat, int param_num)
{
	const char * err = "<unknown>";
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	switch (param_num) {
		case 0: return 111;
		case 1: return 113;
		case 2: return 116;
		case 3: return 117;
		case 4: return 118;
		case 5: return 119;
		case 6: return 120;
		case 7: err = "Downward short-wave radiation flux"; break;
		case 8: err = "Upward short-wave radiation flux"; break;
		case 192:
			switch (center) {
				case 7: return 204;
				default: break;
			}
			break;
		case 193:
			switch (center) {
				case 7: return 211;
				default: break;
			}
			break;
		case 196:
			switch (center) {
				case 7: return 161;
				default: break;
			}
			break;
		default: break;
	}
	fprintf(stderr, "There is no GRIB1 parameter code for '%s' (%s:%d)\n", err, __FILE__, __LINE__);
	return -1;
}

static int map_parameter_meterological_longwaveradiation(int center, int disc, int param_cat, int param_num)
{
	const char * err = "<unknown>";
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	switch (param_num) {
		case 0: return 112;
		case 1: return 114;
		case 2: return 115;
		case 3: err = "Downward long-wave radiation flux"; break;
		case 4: err = "Upward long-wave radiation flux"; break;
		case 192:
			switch (center) {
				case 7: return 205;
				default: break;
			}
			break;
		case 193:
			switch (center) {
				case 7: return 212;
				default: break;
			}
			break;
		default: break;
	}
	fprintf(stderr, "There is no GRIB1 parameter code for '%s' (%s:%d)\n", err, __FILE__, __LINE__);
	return -1;
}

static int map_parameter_meterological_cloud(int center, int disc, int param_cat, int param_num)
{
	const char * err = "<unknown>";
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	switch (param_num) {
		case  0: return 58;
		case  1: return 71;
		case  2: return 72;
		case  3: return 73;
		case  4: return 74;
		case  5: return 75;
		case  6: return 76;
		case  7: err = "Cloud amount"; break;
		case  8: err = "Cloud type"; break;
		case  9: err = "Thunderstorm maximum tops"; break;
		case 10: err = "Thunderstorm coverage"; break;
		case 11: err = "Cloud base"; break;
		case 12: err = "Cloud top"; break;
		case 13: err = "Ceiling"; break;
		case 14: err = "Non-convective cloud cover"; break;
		case 15: err = "Cloud work function"; break;
		case 16: err = "Convective cloud efficiency"; break;
		case 17: err = "Total condensate"; break;
		case 18: err = "Total column-integrated cloud water"; break;
		case 19: err = "Total column-integrated cloud ice"; break;
		case 20: err = "Total column-integrated cloud condensate"; break;
		case 21: err = "Ice fraction of total condensate"; break;
		case 192:
			switch (center) {
				case 7: return 213;
				default: break;
			}
			break;
		case 193:
			switch (center) {
				case 7: return 146;
				default: break;
			}
			break;
		default: break;
	}
	fprintf(stderr, "There is no GRIB1 parameter code for '%s' (%s:%d)\n", err, __FILE__, __LINE__);
	return -1;
}

static int map_parameter_meterological_thermodynamic_stability_index(int center, int disc, int param_cat, int param_num)
{
	const char * err = "<unknown>";
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	switch (param_num) {
		case 0: return 24;
		case 1: return 77;
		case 2: err = "K index"; break;
		case 3: err = "KO index"; break;
		case 4: err = "Total totals index"; break;
		case 5: err = "Sweat index"; break;
		case 6:
			switch (center) {
				case 7: return 157;
				default: err = "Convective available potential energy"; break;
			}
			break;
		case 7:
			switch (center) {
				case 7: return 156;
				default: err = "Convective inhibition"; break;
			}
			break;
		case 8:
			switch (center) {
				case 7: return 190;
				default: err = "Storm-relative helicity"; break;
			}
			break;
		case 9: err = "Energy helicity index"; break;
		case 10: err = "Surface lifted index"; break;
		case 11: err = "Best (4-layer) lifted index"; break;
		case 12: err = "Richardson number"; break;
		case 192:
			switch (center) {
				case 7: return 131;
				default: break;
			}
			break;
		case 193:
			switch (center) {
				case 7: return 132;
				default: break;
			}
			break;
		case 194:
			switch (center) {
				case 7: return 254;
				default: break;
			}
			break;
		default: break;
	}
	fprintf(stderr, "There is no GRIB1 parameter code for '%s' (%s:%d)\n", err, __FILE__, __LINE__);
	return -1;
}

static int map_parameter_meterological_aerosol(int center, int disc, int param_cat, int param_num)
{
	const char * err = "<unknown>";
	UNUSED_ARG(center);
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	switch (param_num) {
		case 0: err = "Aerosol type"; break;
		default: break;
	}
	fprintf(stderr, "There is no GRIB1 parameter code for '%s' (%s:%d)\n", err, __FILE__, __LINE__);
	return -1;
}

static int map_parameter_meterological_tracegas(int center, int disc, int param_cat, int param_num)
{
	const char * err = "<unknown>";
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	switch (param_num) {
		case 0: return 10;
		case 1: err = "Ozone mixing ratio"; break;
		case 192:
			switch (center) {
				case 7: return 154;
				default: break;
			}
			break;
		default: break;
	}
	fprintf(stderr, "There is no GRIB1 parameter code for '%s' (%s:%d)\n", err, __FILE__, __LINE__);
	return -1;
}

static int map_parameter_meterological_radar(int center, int disc, int param_cat, int param_num)
{
	const char * err = "<unknown>";
	UNUSED_ARG(center);
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	switch (param_num) {
		case 0: err = "Base spectrum width"; break;
		case 1: err = "Base reflectivity"; break;
		case 2: err = "Base radial velocity"; break;
		case 3: err = "Vertically-integrated liquid"; break;
		case 4: err = "Layer-maximum base reflectivity"; break;
		case 5: err = "Radar precipitation"; break;
		case 6: return 21;
		case 7: return 22;
		case 8: return 23;
		default: break;
	}
	fprintf(stderr, "There is no GRIB1 parameter code for '%s' (%s:%d)\n", err, __FILE__, __LINE__);
	return -1;
}

static int map_parameter_meterological_nuclear_radiology(int center, int disc, int param_cat, int param_num)
{
	UNUSED_ARG(center);
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	UNUSED_ARG(param_num);
	return -1;
}

static int map_parameter_meterological_physical_atmospheric_property(int center, int disc, int param_cat, int param_num)
{
	const char * err = "<unknown>";
	UNUSED_ARG(disc);
	UNUSED_ARG(param_cat);
	switch (param_num) {
		case  0: return 20;
		case  1: return 84;
		case  2: return 60;
		case  3: return 67;
		case  4: err = "Volcanic ash"; break;
		case  5: err = "Icing top"; break;
		case  6: err = "Icing base"; break;
		case  7: err = "Icing"; break;
		case  8: err = "Turbulence top"; break;
		case  9: err = "Turbulence base"; break;
		case 10: err = "Turbulence"; break;
		case 11: err = "Turbulent kinetic energy"; break;
		case 12: err = "Planetary boundary layer regime"; break;
		case 13: err = "Contrail intensity"; break;
		case 14: err = "Contrail engine type"; break;
		case 15: err = "Contrail top"; break;
		case 16: err = "Contrail base"; break;
		case 17: err = "Maximum snow albedo"; break;
		case 18: err = "Snow-free albedo"; break;
		case 204:
			switch (center) {
				case 7: return 209;
				default: break;
			}
			break;
		default: break;
	}
	fprintf(stderr, "There is no GRIB1 parameter code for '%s' (%s:%d)\n", err, __FILE__, __LINE__);
	return -1;
}

static int map_parameter_meterological(int center, int disc, int param_cat, int param_num)
{
	switch (param_cat) {
		case  0: return map_parameter_meterological_temperature(center, disc, param_cat, param_num);
		case  1: return map_parameter_meterological_moisture(center, disc, param_cat, param_num);
		case  2: return map_parameter_meterological_momentum(center, disc, param_cat, param_num);
		case  3: return map_parameter_meterological_mass(center, disc, param_cat, param_num);
		case  4: return map_parameter_meterological_shortwaveradiation(center, disc, param_cat, param_num);
		case  5: return map_parameter_meterological_longwaveradiation(center, disc, param_cat, param_num);
		case  6: return map_parameter_meterological_cloud(center, disc, param_cat, param_num);
		case  7: return map_parameter_meterological_thermodynamic_stability_index(center, disc, param_cat, param_num);
		case 13: return map_parameter_meterological_aerosol(center, disc, param_cat, param_num);
		case 14: return map_parameter_meterological_tracegas(center, disc, param_cat, param_num);
		case 15: return map_parameter_meterological_radar(center, disc, param_cat, param_num);
		case 18: return map_parameter_meterological_nuclear_radiology(center, disc, param_cat, param_num);
		case 19: return map_parameter_meterological_physical_atmospheric_property(center, disc, param_cat, param_num);
		default: break;
	}
	return -1;
}

static int map_parameter_hydrological(int center, int disc, int param_cat, int param_num)
{
	UNUSED_ARG(disc);
	switch (param_cat) {
		case 0: /* hydrology basic products */
			switch (param_num) {
				case 192:
					switch (center) {
						case 7: return 234;
						default: break;
					}
					break;
				case 193:
					switch (center) {
						case 7: return 235;
						default: break;
					}
					break;
				default: break;
			}
			break;
		case 1:
			switch (param_num) {
				case 192:
					switch (center) {
						case 7: return 194;
						default: break;
					}
				case 193:
					switch (center) {
						case 7: return 195;
						default: break;
					}
				default: break;
			}
			break;
	}
	return -1;
}

static int map_parameter_landsurface(int center, int disc, int param_cat, int param_num)
{
	UNUSED_ARG(disc);
	switch (param_cat) {
		case 0: /* vegetation/biomass */
			switch (param_num) {
				case 0: return 81;
				case 1: return 83;
				case 2: return 85;
				case 3: return 86;
				case 4: return 87;
				case 5: return 90;
				case 192:
					switch (center) {
						case 7: return 144;
						default: break;
					}
				case 193:
					switch (center) {
						case 7: return 155;
						default: break;
					}
				case 194:
					switch (center) {
						case 7: return 207;
						default: break;
					}
				case 195:
					switch (center) {
						case 7: return 208;
						default: break;
					}
				case 196:
					switch (center) {
						case 7: return 223;
						default: break;
					}
				case 197:
					switch (center) {
						case 7: return 226;
						default: break;
					}
				case 198:
					switch (center) {
						case 7: return 225;
						default: break;
					}
				case 207:
					switch (center) {
						case 7: return 201;
						default: break;
					}
				default: break;
			}
			break;
		default: break;
	}
	return -1;
}

static int map_parameter_oceanographic_waves(int center, int disc, int param_cat, int param_num)
{
	UNUSED_ARG(disc);
	UNUSED_ARG(center);
	UNUSED_ARG(param_cat);

	switch (param_num) {
		case  0: return  28;
		case  1: return  29;
		case  2: return  30;
		case  3: return 100;
		case  4: return 101;
		case  5: return 102;
		case  6: return 103;
		case  7: return 104;
		case  8: return 105;
		case  9: return 106;
		case 10: return 107;
		case 11: return 108;
		case 12: return 109;
		case 13: return 110;
		default: break;
	}
	return -1;
}

static int map_parameter_oceanographic_currents(int center, int disc, int param_cat, int param_num)
{
	UNUSED_ARG(disc);
	UNUSED_ARG(center);
	UNUSED_ARG(param_cat);

	switch (param_num) {
		case 0: return 47;
		case 1: return 48;
		case 2: return 49;
		case 3: return 50;
		default: break;
	}
	return -1;
}

static int map_parameter_oceanographic_ice(int center, int disc, int param_cat, int param_num)
{
	UNUSED_ARG(center);
	UNUSED_ARG(disc);
	UNUSED_ARG(center);
	UNUSED_ARG(param_cat);

	switch (param_num) {
		case 0: return 91;
		case 1: return 92;
		case 2: return 93;
		case 3: return 94;
		case 4: return 95;
		case 5: return 96;
		case 6: return 97;
		case 7: return 98;
		default: break;
	}
	return -1;
}

static int map_parameter_oceanographic_surface(int center, int disc, int param_cat, int param_num)
{
	UNUSED_ARG(disc);
	UNUSED_ARG(center);
	UNUSED_ARG(param_cat);

	switch (param_num) {
		case 0: return 80;
		case 1: return 82;
		default: break;
	}
	return -1;
}

static int map_parameter_oceanographic_subsurface(int center, int disc, int param_cat, int param_num)
{
	UNUSED_ARG(disc);
	UNUSED_ARG(center);
	UNUSED_ARG(param_cat);

	switch (param_num) {
		case 0: return 69;
		case 1: return 70;
		case 2: return 68;
		case 3: return 88;
		default: break;
	}
	return -1;
}

static int map_parameter_oceanographic(int center, int disc, int param_cat, int param_num)
{
	UNUSED_ARG(disc);
	UNUSED_ARG(center);

	switch (param_cat) {
		case 0: return map_parameter_oceanographic_waves(center, disc, param_cat, param_num);
		case 1: return map_parameter_oceanographic_currents(center, disc, param_cat, param_num);
		case 2: return map_parameter_oceanographic_ice(center, disc, param_cat, param_num);
		case 3: return map_parameter_oceanographic_surface(center, disc, param_cat, param_num);
		case 4: return map_parameter_oceanographic_subsurface(center, disc, param_cat, param_num);
		default: break;
	}
	return -1;
}

static int map_parameter_data(int center, int disc, int param_cat, int param_num)
{
	switch (disc) {
		case  0: return map_parameter_meterological(center, disc, param_cat, param_num); break;
		case  1: return map_parameter_hydrological(center, disc, param_cat, param_num); break;
		case  2: return map_parameter_landsurface(center, disc, param_cat, param_num); break;
		case 10: return map_parameter_oceanographic(center, disc, param_cat, param_num); break;
		default: break;
	}
	fprintf(stderr,"There is no GRIB1 parameter code for discipline %d, parameter category %d, parameter number %d\n", disc, param_cat, param_num);
	return -1;
}

static int map_level_data(GRIB2Grid * grid, int * level_type, int * level1, int * level2, int center)
{
	if (grid->md.lvl2_type != 255 && grid->md.lvl1_type != grid->md.lvl2_type) {
		fprintf(stderr,"Unable to indicate a layer bounded by different level types %d and %d in GRIB1\n",
			grid->md.lvl1_type, grid->md.lvl2_type);
		return -1;
	}

	*level1 = *level2 = 0;

	switch (grid->md.lvl1_type) {
		case   1: *level_type =  1; break;
		case   2: *level_type =  2; break;
		case   3: *level_type =  3; break;
		case   4: *level_type =  4; break;
		case   5: *level_type =  5; break;
		case   6: *level_type =  6; break;
		case   7: *level_type =  7; break;
		case   8: *level_type =  8; break;
		case   9: *level_type =  9; break;
		case  20: *level_type = 20; break;
		case 100:
			if (grid->md.lvl2_type == 255) {
				*level_type = 100;
				*level1 = grid->md.lvl1 / 100.0;
			} else {
				*level_type = 101;
				*level1 = grid->md.lvl1 / 1000.0;
				*level2 = grid->md.lvl2 / 1000.0;
			}
			break;
		case 101:
			*level_type = 102;
			break;
		case 102:
			if (grid->md.lvl2_type == 255) {
				*level_type = 103;
				*level1 = grid->md.lvl1;
			} else {
				*level_type = 104;
				*level1 = grid->md.lvl1/100.0;
				*level2 = grid->md.lvl2/100.0;
			}
			break;
		case 103:
			if (grid->md.lvl2_type == 255) {
				*level_type = 105;
				*level1 = grid->md.lvl1;
			} else {
				*level_type = 106;
				*level1 = grid->md.lvl1 / 100.0;
				*level2 = grid->md.lvl2 / 100.0;
			}
			break;
		case 104:
			if (grid->md.lvl2_type == 255) {
				*level_type=107;
				*level1=grid->md.lvl1*10000.;
			} else {
				*level_type=108;
				*level1=grid->md.lvl1*100.0;
				*level2=grid->md.lvl2*100.0;
			}
			break;
		case 105:
			*level1=grid->md.lvl1;
			if (grid->md.lvl2_type == 255) {
				*level_type=109;
			} else {
				*level_type=110;
				*level2=grid->md.lvl2;
			}
			break;
		case 106:
			*level1=grid->md.lvl1*100.;
			if (grid->md.lvl2_type == 255) {
				*level_type=111;
			} else {
				*level_type=112;
				*level2=grid->md.lvl2*100.0;
			}
			break;
		case 107:
			if (grid->md.lvl2_type == 255) {
				*level_type=113;
				*level1=grid->md.lvl1;
			} else {
				*level_type=114;
				*level1=475.-grid->md.lvl1;
				*level2=475.-grid->md.lvl2;
			}
			break;
		case 108:
			*level1=grid->md.lvl1/100.;
			if (grid->md.lvl2_type == 255) {
				*level_type=115;
			} else {
				*level_type=116;
				*level2=grid->md.lvl2/100.;
			}
			break;
		case 109:
			*level_type = 117;
			*level1 = grid->md.lvl1 * 1000000000.0;
			break;
		case 111:
			if (grid->md.lvl2_type == 255) {
				*level_type = 119;
				*level1 = grid->md.lvl1*10000.0;
			} else {
				*level_type = 120;
				*level1 = grid->md.lvl1*100.0;
				*level2 = grid->md.lvl2*100.0;
			}
			break;
		case 117:
			fprintf(stderr, "There is no GRIB1 level code for 'Mixed layer depth'\n");
			return -1;
		case 160:
			*level_type = 160;
			*level1 = grid->md.lvl1;
			break;
		case 200:
			switch (center) {
				case 7: *level_type = 200; break;
				default: break;
			}
			break;
		default: break;
	}
	return 0;
}

static int map_time_range(GRIBMessage * msg, GRIB2Grid * grid, int * p1, int * p2, int * t_range, int * n_avg, int * n_missing, int center)
{
	switch (grid->md.pds_templ_num) {
		case 0:
		case 1:
		case 2:
			*t_range = 0;
			*p1 = grid->md.fcst_time;
			*p2 = 0;
			*n_avg = *n_missing = 0;
			break;
		case 8:
		case 11:
		case 12:
			if (grid->md.stat_proc.num_ranges > 1) {
				if (center == 7 && grid->md.stat_proc.num_ranges == 2) {
					/* NCEP CFSR monthly grids */
					*p2 = grid->md.stat_proc.incr_length[0];
					*p1 = *p2-grid->md.stat_proc.time_length[1];
					*n_avg = grid->md.stat_proc.time_length[0];
					switch (grid->md.stat_proc.proc_code[0]) {
						case 193: *t_range = 113; break;
						case 194: *t_range = 123; break;
						case 195: *t_range = 128; break;
						case 196: *t_range = 129; break;
						case 197: *t_range = 130; break;
						case 198: *t_range = 131; break;
						case 199: *t_range = 132; break;
						case 200: *t_range = 133; break;
						case 201: *t_range = 134; break;
						case 202: *t_range = 135; break;
						case 203: *t_range = 136; break;
						case 204: *t_range = 137; break;
						case 205: *t_range = 138; break;
						case 206: *t_range = 139; break;
						case 207: *t_range = 140; break;
						default:
							fprintf(stderr,"Unable to map NCEP statistical process code %d to GRIB1\n",grid->md.stat_proc.proc_code[0]);
							return -1;
					}
				} else {
					fprintf(stderr,"Unable to map multiple statistical processes to GRIB1\n");
					return -1;
				}
			} else {
				switch (grid->md.stat_proc.proc_code[0]) {
					case 0:
					case 1:
					case 4:
						switch (grid->md.stat_proc.proc_code[0]) {
							case 0: *t_range = 3; break; /* average */
							case 1: *t_range = 4; break; /* accumulation */
							case 4: *t_range = 5; break; /* difference */
						}
						*p1 = grid->md.fcst_time;
						*p2 = map_statistical_end_time(msg, grid);
						if (*p2 < 0) {
							return -1;
						}
						if (grid->md.stat_proc.incr_length[0] == 0) {
							*n_avg = 0;
						} else {
							fprintf(stderr, "Unable to map discrete processing to GRIB1\n");
							return -1;
						}
						break;
					case 2: /* maximum */
					case 3: /* minimum */
						*t_range = 2;
						*p1 = grid->md.fcst_time;
						*p2 = map_statistical_end_time(msg, grid);
						if (*p2 < 0) {
							return -1;
						}
						if (grid->md.stat_proc.incr_length[0] == 0) {
							*n_avg = 0;
						} else {
							fprintf(stderr, "Unable to map discrete processing to GRIB1\n");
							return -1;
						}
						break;
					default:
						/* patch for NCEP grids */
						if (grid->md.stat_proc.proc_code[0] == 255 && center == 7) {
							if (msg->disc == 0) {
								if (grid->md.param_cat == 0) {
									switch (grid->md.param_num) {
										case 4:
										case 5:
											*t_range = 2;
											*p1 = grid->md.fcst_time;
											*p2 = map_statistical_end_time(msg, grid);
											if (*p2 < 0) {
												return -1;
											}
											if (grid->md.stat_proc.incr_length[0] == 0) {
												*n_avg=0;
											} else {
												fprintf(stderr,"Unable to map discrete processing to GRIB1\n");
												return -1;
											}
											break;
									}
								}
							}
						} else {
							fprintf(stderr,"Unable to map statistical process %d to GRIB1\n",grid->md.stat_proc.proc_code[0]);
							return -1;
						}
						break;
				}
			}
			*n_missing=grid->md.stat_proc.nmiss;
			break;
		default:
			fprintf(stderr,"Unable to map time range for Product Definition Template %d into GRIB1\n",grid->md.pds_templ_num);
			return -1;
	}
	return 0;
}

int grib2_packPDS(GRIBMessage * msg, int grid_number, unsigned char * grib1_buffer, size_t * offset)
{
	int level_type;
	int level1;
	int level2;
	int p1;
	int p2;
	int t_range;
	int n_avg;
	int n_missing;
	int D;
	int prm;

	static short warned_ensemble = 0;

	/* length of the PDS */
	set_bits(grib1_buffer,28,*offset,24);

	/* GRIB1 tables version number */
	set_bits(grib1_buffer,3,*offset+24,8);

	/* originating center ID */
	set_bits(grib1_buffer,msg->center_id,*offset+32,8);

	/* generating process ID */
	set_bits(grib1_buffer,msg->grids[grid_number].md.gen_proc,*offset+40,8);

	/* grid definition catalog number - set to 255 because GDS is to be included */
	set_bits(grib1_buffer,255,*offset+48,8);

	/* flag */
	if (msg->grids[grid_number].md.bitmap == NULL) {
		set_bits(grib1_buffer,0x80,*offset+56,8);
	} else {
		set_bits(grib1_buffer,0xc0,*offset+56,8);
	}

	/* parameter code */
	prm = map_parameter_data(msg->center_id, msg->disc, msg->grids[grid_number].md.param_cat, msg->grids[grid_number].md.param_num);
	if (prm < 0) {
		return -1;
	}
	set_bits(grib1_buffer, prm, *offset+64,8);
	if (map_level_data(&msg->grids[grid_number], &level_type, &level1, &level2, msg->center_id) != 0) {
		return -1;
	}

	/* level type code */
	set_bits(grib1_buffer, level_type, *offset + 72, 8);
	if (msg->grids[grid_number].md.lvl2_type == 255) {
		set_bits(grib1_buffer,level1,*offset+80,16);
	} else {
		set_bits(grib1_buffer,level1,*offset+80,8);
		set_bits(grib1_buffer,level2,*offset+88,8);
	}

	/* year of century */
	set_bits(grib1_buffer,(msg->yr % 100),*offset+96,8);

	/* month */
	set_bits(grib1_buffer,msg->mo,*offset+104,8);

	/* day */
	set_bits(grib1_buffer,msg->dy,*offset+112,8);

	/* hour */
	set_bits(grib1_buffer,msg->time/10000,*offset+120,8);

	/* minute */
	set_bits(grib1_buffer,(msg->time/100 % 100),*offset+128,8);

	/* second */
	if (msg->md.time_unit == 13) {
		fprintf(stderr,"Unable to indicate 'Second' for time unit in GRIB1\n");
	} else {
		set_bits(grib1_buffer, msg->md.time_unit, *offset + 136, 8);
	}
	if (map_time_range(msg, &msg->grids[grid_number], &p1, &p2, &t_range, &n_avg, &n_missing, msg->center_id) != 0) {
		return -1;
	}
	if (t_range == 10) {
		set_bits(grib1_buffer,p1,*offset+144,16);
	} else {
		set_bits(grib1_buffer,p1,*offset+144,8);
		set_bits(grib1_buffer,p2,*offset+152,8);
	}
	set_bits(grib1_buffer,t_range,*offset+160,8);

	/* century of year */
	set_bits(grib1_buffer,(msg->yr/100)+1,*offset+192,8);

	/* originating sub-center ID */
	set_bits(grib1_buffer,msg->sub_center_id,*offset+200,8);

	/* decimal scale factor */
	D = msg->md.D;
	if (D < 0) {
		D=-D+0x8000;
	}
	set_bits(grib1_buffer, D, *offset + 208, 16);
	*offset += 224;
	if (msg->md.ens_type >= 0) {
		/* length of the PDS */
		set_bits(grib1_buffer,43,*offset-224,24);
		set_bits(grib1_buffer,msg->md.ens_type,*offset+96,8);
		set_bits(grib1_buffer,msg->md.perturb_num,*offset+104,8);
		set_bits(grib1_buffer,msg->md.nfcst_in_ensemble,*offset+112,8);
		(*offset)+=120;
		if (warned_ensemble == 0) {
			fprintf(stderr,"Notice: the 'Ensemble type code', the 'Perturbation Number', and the\n");
			fprintf(stderr,"'Number of forecasts in ensemble' from Product Definition Template 4.1 and/or\n");
			fprintf(stderr,"Product Definition Template 4.12 have been packed in octets 41, 42, and 43 of\n");
			fprintf(stderr,"the GRIB1 Product Definition Section\n");
			warned_ensemble=1;
		}
	} else if (msg->md.derived_fcst_code >= 0) {
		/* length of the PDS */
		set_bits(grib1_buffer,42,*offset-224,24);
		set_bits(grib1_buffer,msg->md.derived_fcst_code,*offset+96,8);
		set_bits(grib1_buffer,msg->md.nfcst_in_ensemble,*offset+104,8);
		(*offset)+=112;
		if (warned_ensemble == 0) {
			fprintf(stderr,"Notice: the 'Derived forecast code' and the 'Number of forecasts in ensemble'\n");
			fprintf(stderr,"from Product Definition Template 4.2 and/or Product Definition Template 4.12\n");
			fprintf(stderr,"have been packed in octets 41 and 42 of the GRIB1 Product Definition Section\n");
			warned_ensemble=1;
		}
	}
	return 0;
}

int grib2_packGDS(GRIBMessage * msg, int grid_number, unsigned char * grib1_buffer, size_t * offset)
{
	int rescomp = 0;
	int value;

	UNUSED_ARG(grid_number);

	/* NV */
	set_bits(grib1_buffer,255,*offset+24,8);
	/* PV */
	set_bits(grib1_buffer,255,*offset+32,8);
	switch (msg->md.gds_templ_num) {
		case 0:
			/* length of the GDS */
			set_bits(grib1_buffer,32,*offset,24);

			/* data representation */
			set_bits(grib1_buffer,0,*offset+40,8);

			/* Ni */
			set_bits(grib1_buffer,msg->md.nx,*offset+48,16);

			/* Nj */
			set_bits(grib1_buffer,msg->md.ny,*offset+64,16);

			/* first latitude */
			value=msg->md.slat*1000.;
			if (value < 0.0) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+80,1);
				set_bits(grib1_buffer,value,*offset+81,23);
			} else {
				set_bits(grib1_buffer,value,*offset+80,24);
			}

			/* first longitude */
			value=msg->md.slon*1000.;
			if (value < 0.0) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+104,1);
				set_bits(grib1_buffer,value,*offset+105,23);
			} else {
				set_bits(grib1_buffer,value,*offset+104,24);
			}

			/* resolution and component flags */
			if ((msg->md.rescomp & 0x20) == 0x20) {
				rescomp|=0x80;
			}
			if (msg->md.earth_shape == 2) {
				rescomp|=0x40;
			}
			if ((msg->md.rescomp & 0x8) == 0x8) {
				rescomp|=0x8;
			}
			set_bits(grib1_buffer,rescomp,*offset+128,8);

			/* last latitude */
			value=msg->md.lats.elat*1000.;
			if (value < 0.0) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+136,1);
				set_bits(grib1_buffer,value,*offset+137,23);
			} else {
				set_bits(grib1_buffer,value,*offset+136,24);
			}

			/* last longitude */
			value=msg->md.lons.elon*1000.;
			if (value < 0.0) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+160,1);
				set_bits(grib1_buffer,value,*offset+161,23);
			} else {
				set_bits(grib1_buffer,value,*offset+160,24);
			}

			/* Di increment */
			value=msg->md.xinc.loinc*1000.;
			if (value < 0.0) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+184,1);
				set_bits(grib1_buffer,value,*offset+185,15);
			} else {
				set_bits(grib1_buffer,value,*offset+184,16);
			}

			/* Dj increment */
			value=msg->md.yinc.lainc*1000.;
			if (value < 0.) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+200,1);
				set_bits(grib1_buffer,value,*offset+201,15);
			} else {
				set_bits(grib1_buffer,value,*offset+200,16);
			}

			/* scanning mode */
			set_bits(grib1_buffer,msg->md.scan_mode,*offset+216,8);

			/* reserved */
			set_bits(grib1_buffer,0,*offset+224,32);
			(*offset)+=256;
			break;

		case 30:
			/* length of the GDS */
			set_bits(grib1_buffer,42,*offset,24);

			/* data representation */
			set_bits(grib1_buffer,3,*offset+40,8);

			/* Nx */
			set_bits(grib1_buffer,msg->md.nx,*offset+48,16);

			/* Ny */
			set_bits(grib1_buffer,msg->md.ny,*offset+64,16);

			/* first latitude */
			value=msg->md.slat*1000.;
			if (value < 0.) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+80,1);
				set_bits(grib1_buffer,value,*offset+81,23);
			} else {
				set_bits(grib1_buffer,value,*offset+80,24);
			}

			/* first longitude */
			value=msg->md.slon*1000.;
			if (value < 0.) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+104,1);
				set_bits(grib1_buffer,value,*offset+105,23);
			} else {
				set_bits(grib1_buffer,value,*offset+104,24);
			}

			/* resolution and component flags */
			if ((msg->md.rescomp & 0x20) == 0x20) {
				rescomp|=0x80;
			}
			if (msg->md.earth_shape == 2) {
				rescomp|=0x40;
			}
			if ((msg->md.rescomp & 0x8) == 0x8) {
				rescomp|=0x8;
			}
			set_bits(grib1_buffer,rescomp,*offset+128,8);

			/* LoV */
			value=msg->md.lons.lov*1000.;
			if (value < 0.) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+136,1);
				set_bits(grib1_buffer,value,*offset+137,23);
			} else {
				set_bits(grib1_buffer,value,*offset+136,24);
			}

			/* Dx */
			value=msg->md.xinc.dxinc+0.5;
			set_bits(grib1_buffer,value,*offset+160,24);

			/* Dy */
			value=msg->md.yinc.dyinc+0.5;
			set_bits(grib1_buffer,value,*offset+184,24);

			/* projection center flag */
			set_bits(grib1_buffer,msg->md.proj_flag,*offset+208,8);

			/* scanning mode */
			set_bits(grib1_buffer,msg->md.scan_mode,*offset+216,8);

			/* latin1 */
			value=msg->md.latin1*1000.;
			if (value < 0.) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+224,1);
				set_bits(grib1_buffer,value,*offset+225,23);
			} else {
				set_bits(grib1_buffer,value,*offset+224,24);
			}

			/* latin2 */
			value=msg->md.latin2*1000.;
			if (value < 0.) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+248,1);
				set_bits(grib1_buffer,value,*offset+249,23);
			} else {
				set_bits(grib1_buffer,value,*offset+248,24);
			}

			/* latitude of southern pole of projection */
			value = msg->md.splat * 1000.0;
			if (value < 0.) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+272,1);
				set_bits(grib1_buffer,value,*offset+273,23);
			} else {
				set_bits(grib1_buffer,value,*offset+272,24);
			}

			/* longitude of southern pole of projection */
			value = msg->md.splon * 1000.0;
			if (value < 0.) {
				value=-value;
				set_bits(grib1_buffer,1,*offset+296,1);
				set_bits(grib1_buffer,value,*offset+297,23);
			} else {
				set_bits(grib1_buffer,value,*offset+296,24);
			}

			/* reserved */
			set_bits(grib1_buffer,0,*offset+320,16);
			(*offset)+=336;
			break;

		default:
			fprintf(stderr,"Unable to map Grid Definition Template %d into GRIB1\n",msg->md.gds_templ_num);
			return -1;
	}
	return 0;
}

int grib2_packBMS(GRIBMessage * msg, int grid_number, unsigned char * grib1_buffer, size_t * offset, size_t num_points)
{
	int length = 6 + (num_points + 7) / 8;
	int ub = 8 - (num_points % 8);
	size_t n;
	size_t off;

	/* length of the BMS */
	set_bits(grib1_buffer,length,*offset,24);

	/* unused bits at end of section */
	set_bits(grib1_buffer,ub,*offset+24,8);

	/* table reference */
	set_bits(grib1_buffer,0,*offset+32,16);

	/* the bitmap */
	off = *offset + 48;
	for (n = 0; n < num_points; n++) {
		set_bits(grib1_buffer,msg->grids[grid_number].md.bitmap[n],off++,1);
	}
	*offset += length * 8;

	return 0;
}

int grib2_packBDS(GRIBMessage * msg, int grid_number, unsigned char * grib1_buffer, size_t * offset, int * pvals, size_t num_to_pack, int pack_width)
{
	int length = 11 + (num_to_pack * pack_width + 7) / 8;
	size_t m;
	size_t off;
	int E;
	int ibm_rep;

	/* length of the BDS */
	set_bits(grib1_buffer, length, *offset, 24);

	/* flag */
	set_bits(grib1_buffer, 0, *offset + 24, 4);

	/* unused bits */
	set_bits(grib1_buffer, (length - 11) * 8 - (num_to_pack * pack_width), *offset + 28, 4);

	/* scale factor E */
	E = msg->grids[grid_number].md.E;
	if (E < 0) {
		E = -E + 0x8000;
	}
	set_bits(grib1_buffer, E, *offset + 32, 16);

	/* Reference value */
	ibm_rep = ieee2ibm(msg->grids[grid_number].md.R * pow(10.0, msg->grids[grid_number].md.D));
	memcpy(&grib1_buffer[(*offset+48)/8], &ibm_rep, 4);

	/* width in bits of each packed value */
	set_bits(grib1_buffer, pack_width, *offset + 80, 8);

	/* packed data values */
	off = *offset + 88;
	for (m = 0; m < num_to_pack; m++) {
		set_bits(grib1_buffer, pvals[m], off, pack_width);
		off += pack_width;
	}

	return 0;
}

