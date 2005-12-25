#include "common.h"
#include <stdio.h>
int main()
{
extern struct q3c_prm hprm;

q3c_ipix_t arr[10],ipix;

q3c_coord_t ra=80.070114,dec=-9.884149;
q3c_coord_t ra1,dec1;

q3c_coord_t rad=1.;

for(ra=0.;ra<=359.;ra+=.1)
for(dec=-90.;dec<=90;dec+=.1)
{
q3c_ang2ipix(&hprm,ra,dec,&ipix);
q3c_ipix2ang(&hprm,ipix,&ra1,&dec1);
//if ((abs(ra-ra1)>0.1)||(abs(dec-dec1)>0.1))
//fprintf(stdout,""Q3C_COORD_FMT" "Q3C_COORD_FMT": "Q3C_COORD_FMT" "Q3C_COORD_FMT" "Q3C_COORD_FMT" "Q3C_COORD_FMT"\n",ra,dec,ra-ra1,dec-dec1,ra1,dec1);
}
return 0;
}
