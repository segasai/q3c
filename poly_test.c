#include "common.h"
#include <stdio.h>

int main(int argc ,char *argv[])
{
  extern struct q3c_prm hprm;

  struct q3c_poly qp;
  q3c_ipix_t xx2[30000];
  q3c_ipix_t xx3[30000];

/*  
  double x0, y0;

x0 = atof (argv[1]);
  y0 = atof (argv[2]);
  */
/*
  q3c_coord_t xc_cur, yc_cur, cur_size;
  if (argc!=4) {fprintf(stderr,"no args!\n");exit(1);}  

  xc_cur = atof (argv[1]);
  yc_cur = atof (argv[2]);
  cur_size = atof (argv[3]);


*/
  q3c_init_poly(&qp,3);
  qp.ra[0]=44; 
  qp.dec[0]=60;
  qp.ra[1]=46; 
  qp.dec[1]=60;
  qp.ra[2]=45; 
  qp.dec[2]=63;


/*
  q3c_init_poly(&qp,4);

  qp.x[0]=0;
  qp.x[1]=0;
  qp.x[2]=1;
  qp.x[3]=1;
  qp.y[0]=0;
  qp.y[1]=1;
  qp.y[2]=1;
  qp.y[3]=0;
*/
  q3c_prepare_poly(&qp);

/*
  fprintf( stderr, "%d\n", q3c_check_point_in_poly(&qp, x0, y0));
  q3c_coord_t xmi, xma, ymi, yma;

  q3c_get_minmax_poly(&qp, &xmi, &xma, &ymi, &yma);
*/
//  fprintf(stderr," %f %f %f %f\n", xmi, xma, ymi, yma);
  
  q3c_poly_query(&hprm, &qp, xx2, xx3);
 //fprintf (stderr, "%d\n",q3c_poly_cover_check(&qp,xc_cur,yc_cur,cur_size));
                                                                                
  return 0;
  

}

