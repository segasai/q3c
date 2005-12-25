#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
/*#include "my_bits.h"*/
#include "common.h"
int main()
{
extern struct q3c_prm hprm;
q3c_ipix_t ipix,ipix1;
q3c_coord_t ra,dec,ra1,dec1,rad;
struct timeval tv1,tv2;
char qstring[20001];
gettimeofday(&tv1,NULL);


//init_q3c1(&hprm1,1024); 

//init_q3c1(&hprm1,33554432);

//init_q3c1(&hprm,1073741824);

gettimeofday(&tv2,NULL);
//fprintf(stdout,"TIME1: %f",tv2.tv_sec-tv1.tv_sec+(tv2.tv_usec-tv1.tv_usec)/1000000.);

ra=56L; dec=-64.L;
ra=0L; dec=-47.L;


gettimeofday(&tv1,NULL);
//for(;!feof(stdin);) {
//fscanf(stdin,"%Lf %Lf",&ra,&dec);

/*
for(ra=0;   ra <= 360; ra+=0.1)
for(dec= -90;dec <= 90;dec+=0.1)
{
ang2ipix(&hprm1, ra,dec,&ipix);
ipix2ang(&hprm1, ipix,&ra1,&dec1);
}
*/

/*
for(ipix=1000000000000000000LL;ipix<=1000000000000000000LL;ipix++)
{
ipix2ang(&hprm1, ipix,&ra1,&dec1);
ang2ipix(&hprm1, ra1,dec1,&ipix1);
}
*/

//ang2ipix(&hprm1, ra,dec,&ipix);
//ipix2ang(&hprm1, ipix,&ra,&dec);
//ang2ipix(&hprm1, ra,dec,&ipix);
//ang2ipix(&hprm1, ra,dec,&ipix1);
//}


rad=0.2L;
//radial_query(&hprm,"q3c",ra,dec,rad,qstring);

q3c_coord_t axx, ayy, axy, ax, ay, a,xmin,xmax,ymin,ymax,x1,x2,y1,y2;
char face_num;
q3c_ipix_t ipix_buf[2];
int i=0;
//for(ra=0;   ra <= 47; ra+=0.05)
//{//fprintf(stderr,"%Lf\n",ra);
//for(dec= -89.99;dec <= -70;dec+=0.05)
//for(ra=0;   ra <= 47; ra+=0.02)
{//fprintf(stderr,"%Lf\n",ra);
//for(dec= -30.99;dec <= -0;dec+=0.02)

{
i++;
//  ra=315.47241973876953125;
//  dec=-73.4825897216796875;
//radial_query(&hprm,"q3c",ra,dec,rad,qstring);
//fprintf(stdout,"%s", qstring);

//  face_num=q3c_getfacenum(ra,dec);
//  get_poly_coefs(face_num,ra,dec,rad,&axx,&ayy,&axy,&ax,&ay,&a);
//  get_xy_minmax(axx,ayy,axy,ax,ay,a,&xmin,&xmax,&ymin,&ymax);
//  fprintf(stdout, "%Lf %Lf %Lf %Lf %Lf %Lf ",ra,dec,xmin,xmax,ymin,ymax);
  x1=xmin;x2=xmax;y1=ymin;y2=ymax;
// q3c_fast_get_xy_minmax(face_num,ra,dec,rad,&xmin,&xmax,&ymin,&ymax);
//  fprintf(stdout, "%Lf %Lf %Lf %Lf\n",xmin,xmax,ymin,ymax);


//ang2ipix(&hprm, ra,dec,&ipix);
// q3c_get_nearby_split(&hprm, ra,dec, rad, ipix_buf,1);

// q3c_get_nearby(&hprm, ra,dec, rad, ipix_buf);

// q3c_get_nearby_split(&hprm, ra,dec, rad, ipix_buf,2);
// q3c_get_nearby_split(&hprm, ra,dec, rad, ipix_buf,3);
// q3c_get_nearby_split(&hprm, ra,dec, rad, ipix_buf,4);

  //fprintf(stdout, "%Lf %Lf %Lf %Lf %Lf %Lf\n",ra,dec,xmin-x1,xmax-x2,ymin-y1,ymax-y2);

}
}


for(i=0;i<=1000000;i++)
{
  q3c_ang2ipix(&hprm, ra, dec , &ipix);
}

gettimeofday(&tv2,NULL);
fprintf(stdout,"N: %d\n",i);
//fprintf(stdout,"TIME2: %f",tv2.tv_sec-tv1.tv_sec+(tv2.tv_usec-tv1.tv_usec)/1000000.);

return 0;
}
