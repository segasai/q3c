/*
	   Copyright (C) 2004-2006 Sergey Koposov
   
    Author: Sergey Koposov, Sternberg Astronomical Institute
    Email: math@sai.msu.ru 
    http://lnfm1.sai.msu.ru/~math

    This file is part of Q3C.

    Q3C is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Q3C is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Q3C; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "common.h"

void q3c_init_poly(q3c_poly *qp, int n)
{
	qp->ra = malloc(n * sizeof(q3c_coord_t));
	qp->dec = malloc(n * sizeof(q3c_coord_t));

	qp->ax = malloc(n * sizeof(q3c_coord_t));
	qp->ay = malloc(n * sizeof(q3c_coord_t));

	qp->x = malloc(n * sizeof(q3c_coord_t));
	qp->y = malloc(n * sizeof(q3c_coord_t));
	qp->n = n;
}

void q3c_prepare_poly(q3c_poly *qp)
{
	int n = qp->n - 1 ;
	int i;
	q3c_coord_t *ax = qp->ax;
	q3c_coord_t *ay = qp->ay;
	q3c_coord_t *x = qp->x;
	q3c_coord_t *y = qp->y;

	for(i = 0 ; i < n; i++)
	{
		ax[i] = x[i + 1] - x[i];
		ay[i] = y[i + 1] - y[i];
	}
	ax[i] = x[0] - x[i];
	ay[i] = y[0] - y[i];
}


int q3c_check_point_in_poly(q3c_poly *qp, q3c_coord_t x0,
                            q3c_coord_t y0)
{
	int i, n = qp->n;
	q3c_coord_t *ax = qp->ax;
	q3c_coord_t *ay = qp->ay;
	q3c_coord_t *x = qp->x;
	q3c_coord_t *y = qp->y;
	
	q3c_coord_t sign0, sign1, sign2, sign3;
	
	sign0 = (y0 - y[0]) * ax[0] - (x0 - x[0]) * ay[0];   
	
	if (sign0 == 0) 
	{
		sign1 = (y0 - y[1]) * ax[1] - (x0 - x[1]) * ay[1];   
		sign2 = (y0 - y[n - 1]) * ax[n - 1] - (x0 - x[n - 1]) * ay[n - 1];
		sign3 = sign1 * sign2; 
		if (sign3 >= 0)
		{
			/* on border */
			return q3c_EDGE;
		}
		else
		{
			/* outside */
			return q3c_DISJUNCT;
		}
	}
	else if (sign0 > 0)
	{
		for(i = 1 ; i < n ; i++)
		{
			sign1 = (y0 - y[i]) * ax[i] - (x0 - x[i]) * ay[i];
			if (sign1 <= 0)
			{
				if (sign1 < 0)
				{
					/* outside */
					return q3c_DISJUNCT;
				}
				else
				{
					sign2 = (y0 - y[(i + 1) % n]) * ax [(i + 1) % n] -
						(x0 - x[(i + 1) % n]) * ay[(i + 1) % n];
					if (sign2 >= 0)
					{
						/* on border */
						return q3c_EDGE;
					}
					else
					{
						/* outside */
						return q3c_DISJUNCT;
					}
				}
			}
		}
		/* inside */
		return q3c_COVER;
	}
	else
	{
		for(i = 1 ; i < n ; i++)
		{
			sign1 = (y0 - y[i]) * ax[i] - (x0 - x[i]) * ay[i];
			if (sign1 >= 0)
			{
				if (sign1 > 0)
				{
					/* outside */
					return q3c_DISJUNCT;
				}
				else
				{
					sign2 = (y0 - y[(i + 1) % n]) * ax [(i + 1) % n] -
						(x0 - x[(i + 1) % n]) * ay[(i + 1) % n];
					if (sign2 <= 0)
					{
						/* on border */
						return q3c_EDGE;
					}
					else
					{
						/* outside */
						return q3c_DISJUNCT;
					}
				}
			}
		}
		/* inside */
		return q3c_COVER;
	}
}


void q3c_get_minmax_poly(q3c_poly *qp, q3c_coord_t *xmin,
                         q3c_coord_t *xmax, q3c_coord_t *ymin, 
                         q3c_coord_t *ymax)
{
	int i;
	const int n = qp->n;
	q3c_coord_t *x = qp->x, *y = qp->y, t;
	q3c_coord_t xmi, xma, ymi, yma;
	
	xmi = x[0];
	xma = x[0];
	ymi = y[0];
	yma = y[0];
	
	for(i = 1; i < n; i++)
	{
		t = x[i];
		if (t > xma)
		{
			xma = t;
		}
		else if (t < xmi)
		{
			xmi = t;
		}
		
		t = y[i];
		
		if (t > yma)
		{
			yma = t;
		}
		else if (t < ymi)
		{
			ymi = t;
		}
	}
	
	*xmin = xmi;
	*xmax = xma;
	*ymin = ymi;
	*ymax = yma;
}


char q3c_get_facenum_poly(q3c_poly *qp)
{
	return q3c_get_facenum(qp->ra[0], qp->dec[0]);
}

void q3c_project_poly(q3c_poly *qp, char face_num)
{
	q3c_coord_t ra1, dec1, tmp0;
	q3c_coord_t *ra = qp->ra, *dec = qp->dec;
	q3c_coord_t *x = qp->x, *y = qp->y, x0, y0;
	
	int i, n = qp->n;
	if ((face_num > 0) && (face_num < 5))
	{
		face_num--; /* Just computation trick */
		for (i = 0; i < n; i++)
		{
			ra1 = q3c_DEGRA * (ra[i] - 90 * (q3c_coord_t)face_num);
			dec1 = q3c_DEGRA * dec[i];
			x[i] = (q3c_tan(ra1)) / 2;
			y[i] = (q3c_tan(dec1) / q3c_cos(ra1)) / 2;
		}
		/* Now x[i] and y[i] are coordinates on cube face [-0.5:0.5]x[-0.5:0.5] */
	}
	else if (face_num == 0)
	{
		for (i = 0; i < n; i++)
		{
			ra1 = q3c_DEGRA * ra[i];
			dec1 = q3c_DEGRA * dec[i];
			
			tmp0 = 1 / q3c_tan(dec1);
#ifdef __USE_GNU
			q3c_sincos(ra1, &x0, &y0);
#else
			x0 = q3c_sin(ra1);
			y0 = q3c_cos(ra1);
#endif
			x0 *= tmp0;
			y0 *= (-tmp0);
			x[i] = x0 / 2;
			y[i] = y0 / 2;
		}
	} 
	else 
	{
		for (i = 0; i < n; i++)
		{
			ra1 = q3c_DEGRA * ra[i];
			dec1 = q3c_DEGRA * dec[i];

			tmp0 = 1 / q3c_tan(dec1);
#ifdef __USE_GNU
			q3c_sincos(ra1, &x0, &y0);
#else
			x0 = q3c_sin(ra1);
			y0 = q3c_cos(ra1);
#endif
			x0 *= (-tmp0);
			y0 *= (-tmp0);
			x[i] = x0 / 2;
			y[i] = y0 / 2;
		}
	}
}


static char q3c_poly_intersection_check(q3c_poly *qp,
	q3c_coord_t xl, q3c_coord_t xr,
	q3c_coord_t yb, q3c_coord_t yt,
	q3c_coord_t cur_size) 
{
  int i, n = qp->n;
  q3c_coord_t *ax = qp->ax; 
  q3c_coord_t *ay = qp->ay; 
  q3c_coord_t *x = qp->x; 
  q3c_coord_t *y = qp->y; 
  q3c_coord_t txl, txr, tyb, tyt, axi, ayi, xi, yi, tmp, tmp1;
  char ret = 0;
  for( i = 0; i <n ; i++)
  {
    xi = x[i];
    yi = y[i];
    axi = ax[i];
    ayi = ay[i];
    txl = xl - xi;
    txr = xr - xi;
    tyb = yb - yi;
    tyt = yt - yi;

    tmp = tyb / ayi;
    tmp1 = axi * tmp - txl;
    if ((tmp >= 0) && (tmp <= 1) && (tmp1 >= 0) && (tmp1 <= cur_size)) 
    {	
      ret = 1;
      break;
    }    

    tmp = tyt / ayi;
    tmp1 = axi * tmp - txl;
    if ((tmp >= 0) && (tmp <= 1) && (tmp1 >= 0) && (tmp1 <= cur_size)) 
    {
      ret = 1;
      break;
    }    

    tmp = txl / axi;
    tmp1 = ayi * tmp - tyb;
    if ((tmp >= 0) && (tmp <= 1) && (tmp1 >= 0) && (tmp1 <= cur_size))
    {
      ret = 1;
      break;
    }    

    tmp = txr / axi;
    tmp1 = ayi * tmp - tyb;
    if ((tmp >= 0) && (tmp <= 1) && (tmp1 >= 0) && (tmp1 <= cur_size)) 
    {
      ret = 1;
      break;
    }    
  }

  return ret;
}



int q3c_poly_cover_check(q3c_poly *qp, q3c_coord_t xc_cur, 
                         q3c_coord_t yc_cur, q3c_coord_t cur_size)
{
    q3c_coord_t xl_cur, xr_cur, yb_cur, yt_cur;
    int val;


    /* Checking the intersection of ellipse and box
     * The box parameters are set by variables xc_cur, yc_cur and cur_size 
     */                                                                    
    xl_cur = xc_cur - cur_size / 2; /* left   */
    xr_cur = xc_cur + cur_size / 2; /* right  */
    yb_cur = yc_cur - cur_size / 2; /* bottom */ 
    yt_cur = yc_cur + cur_size / 2; /* top    */

    /* Undef labels -- the labels when the current computed values dont allow
     * to make the final decision about the intersection
     */

    val = q3c_check_point_in_poly(qp, xl_cur, yb_cur);
    if (val != q3c_DISJUNCT)
    {
      goto PARTUNDEF_CHECK01;
    }
    
    val = q3c_check_point_in_poly(qp, xr_cur, yb_cur);
    if (val != q3c_DISJUNCT)
    {
      return q3c_PARTIAL;
    }
        
    val = q3c_check_point_in_poly(qp, xr_cur, yt_cur);
    if (val != q3c_DISJUNCT)
    {
      return q3c_PARTIAL;
    }

    val = q3c_check_point_in_poly(qp, xl_cur, yt_cur);

    if (val != q3c_DISJUNCT)
    {
      return q3c_PARTIAL;
    }
    else 
    {
      if (q3c_poly_intersection_check(qp, xl_cur, xr_cur, yb_cur, yt_cur, cur_size)||
           ((qp->x[0] > xl_cur) && (qp->x[0] < xr_cur) && 
            (qp->y[0] > yb_cur) && (qp->y[0] < yt_cur)))
      {
        return q3c_PARTIAL;
      }
      else
      {
        return q3c_DISJUNCT;
      }
    }


    PARTUNDEF_CHECK01:
    val = q3c_check_point_in_poly(qp, xr_cur, yb_cur);
    if (val == q3c_DISJUNCT)
    {
      return q3c_PARTIAL;    
    }
      
    //PARTUNDEF_CHECK11:
    val = q3c_check_point_in_poly(qp, xr_cur, yt_cur);
    if (val == q3c_DISJUNCT)
    {
      return q3c_PARTIAL;    
    }


    //PARTUNDEF_CHECK10:
    val = q3c_check_point_in_poly(qp, xl_cur, yt_cur);
    if (val == q3c_DISJUNCT)
    {
      return q3c_PARTIAL;    
    }
    else
    {
      return q3c_COVER;
    }
}


int q3c_check_sphere_point_in_poly(struct q3c_prm *hprm, int n,
                                   q3c_coord_t in_ra[], q3c_coord_t in_dec[],
                                   q3c_coord_t ra0, q3c_coord_t dec0,
                                   int invocation)
{
  q3c_coord_t xmin,xmax,ymin, ymax;
  static char faces[6], multi_flag;
  q3c_ipix_t ipix;
  q3c_coord_t points[4];
  char face_num, face_num0, cur_face_num;
#define max_n_poly 100
  static q3c_coord_t x[3][max_n_poly], y[3][max_n_poly], ax[3][max_n_poly],
                     ay[3][max_n_poly], x0, y0;
#undef max_n_poly
  int face_count = -1, i;
  

  q3c_poly qp;
  
  ang2ipix_xy(hprm, ra0, dec0, &cur_face_num, &ipix, &x0, &y0);

  qp.ra = in_ra;
  qp.dec = in_dec;  
  qp.n = n;
  
  if (invocation == 0)
  { 
    face_num = q3c_get_facenum_poly(&qp);
    faces[0] = face_num;

    qp.x = x[0];
    qp.y = y[0];
    qp.ax = ax[0];
    qp.ay = ay[0];
    

    q3c_project_poly(&qp, face_num);
    q3c_prepare_poly(&qp);
    
    q3c_get_minmax_poly(&qp, &xmin, &xmax, &ymin, &ymax);
    

    /* Now in a little bit ugly but fastest way I determine whether the ellipse 
     * intersect other faces or not, and if yes, I setup the array "points" to the
     * multi_face loop.
     */
    if (xmin < -q3c_HALF)
    {
      if (ymin < -q3c_HALF)
      {
        points[0] = xmax;
        points[1] = ymin;
        points[2] = xmin;
        points[3] = ymax;
        multi_flag = 2;
        xmin = -q3c_HALF;
        ymin = -q3c_HALF;
      }
      else 
      {
        if (ymax > q3c_HALF)
        {
          points[0] = xmax;
          points[1] = ymax;
          points[2] = xmin;
          points[3] = ymin;
          multi_flag = 2;
          xmin = -q3c_HALF;
          ymax = q3c_HALF;
        }
        else
        {
          points[0] = xmin;
          points[1] = (ymin + ymax) / 2;
          multi_flag = 1;      
          xmin = -q3c_HALF;
        }
      }
    }
    else 
    {
      if (xmax > q3c_HALF)
      {
        if (ymin < -q3c_HALF)
        {
          points[0] = xmin;
          points[1] = ymin;
          points[2] = xmax;
          points[3] = ymax;
          multi_flag = 2;      
          xmax = q3c_HALF;
          ymin = -q3c_HALF;
        }
        else
        {
          if (ymax > q3c_HALF)
          {
            points[0] = xmin;
            points[1] = ymax;
            points[2] = xmax;
            points[3] = ymax;
            multi_flag = 2;
            xmax = q3c_HALF;
            ymax = q3c_HALF;
          }
          else
          {
            points[0] = xmax;
            points[1] = (ymin + ymax) / 2;
            multi_flag = 1;
            xmax = q3c_HALF;
          }
        }
      }
      else
      {
        if (ymin < -q3c_HALF)
        {
          points[0] = (xmin + xmax) / 2;
          points[1] = ymin;
          multi_flag = 1;
          ymin = -q3c_HALF;
        }
        else 
        {
          if (ymax > q3c_HALF)
          {
            points[0] = (xmin + xmax) / 2;
            points[1] = ymax;
            multi_flag = 2;
            ymax = q3c_HALF;
          }
          else
          {
            multi_flag = 0;
          }
        }
      }
    }

    face_num0 = face_num;
    
    for(face_count = 0; face_count <= multi_flag; face_count++)
    {
      /* This the beginning of the mega-loop over multiple faces */
      
      
      if (face_count > 0)
        /* This "if" works when we pass through the secondary faces */ 
      {
        face_num = q3c_xy2facenum(2 * points[2 * face_count - 2],
                      2 * points[2 * face_count - 1], face_num0);
              
        faces[face_count] = face_num;

        qp.x = x[face_count];
        qp.y = y[face_count];
        qp.ax = ax[face_count];
        qp.ay = ay[face_count];
        
        q3c_project_poly(&qp, faces[face_count]);

        q3c_prepare_poly(&qp);
      }      
    }
  }


  for (i = 0; i <= multi_flag; i++)
  {
    if (faces[i] == cur_face_num)
    {
      face_count = i;
      break;
    }
  }

  if (i == (multi_flag + 1))
  {
    return 0;
  }

  qp.x = x[face_count];
  qp.y = y[face_count];
  qp.ax = ax[face_count];
  qp.ay = ay[face_count];


  return q3c_check_point_in_poly(&qp, x0, y0);
}
