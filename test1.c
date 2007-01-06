#include <stdio.h>
#include <stdlib.h>
#include "common.h"
int main()
{
extern struct q3c_prm hprm;
q3c_coord_t ra,dec,radius;
q3c_ipix_t ipix[2];
int i;
q3c_ipix_t xx2[30000];
q3c_ipix_t xx3[30000];
char xx[30000];

//ra=0; dec=0; radius=1;
//ra=2; dec=3; radius= 0.4;
//ra=0; dec=10; radius= 20;

i=1;

//for(i=1;i<=4;i++)
{
///q3c_radial_query(&hprm,"",ra,dec,radius,xx);
//q3c_get_nearby(&hprm,ra,dec,radius, ipix);
//fprintf(stdout, "%.16f", q3c_dist(0,-90,0,-89));
//q3c_radial_query(&hprm,"","","",ra,dec,radius,xx);

//ra=45; dec=35; radius= 1;

ra=10 ;
dec=-70;
radius=10;
double ell = 0.;
double pa = 0;
q3c_ipix_t xx[8];
//q3c_new_radial_query(&hprm,ra,dec,radius,xx2,xx3);
q3c_ellipse_query(&hprm,ra,dec,radius,ell,pa,xx2,xx3);

q3c_circle_region circle;
circle.ra=ra;
circle.dec=dec;
circle.rad=radius;
//q3c_get_nearby(&hprm, Q3C_CIRCLE, &circle, xx);

//  fprintf(stderr,"%lld %lld\n",ipix[0],ipix[1]);

}

return 0;
}
