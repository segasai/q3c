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

#define _GNU_SOURCE
#include "common.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "my_bits.h"

inline q3c_coord_t q3c_dist (q3c_coord_t ra1, q3c_coord_t dec1,
                            q3c_coord_t ra2, q3c_coord_t dec2)
{
  /* The FASTEST!!! (and precise) way to compute the distance on the sphere
   * it uses just 3 !!! computation of trigonometric functions
   */

  q3c_coord_t x = q3c_sin ((ra1 - ra2) / 2 * q3c_DEGRA );
  x *= x;
  q3c_coord_t y = q3c_sin ((dec1 - dec2) / 2 * q3c_DEGRA);
  y *= y;

  /* Seem to be more precise :) */
  q3c_coord_t z = q3c_cos ((dec1 + dec2)/2 * q3c_DEGRA);
  z*=z;

  return 2 * q3c_asin (q3c_sqrt (x * (z - y) + y)) * q3c_RADEG;
}


inline q3c_coord_t q3c_sindist (q3c_coord_t ra1, q3c_coord_t dec1,
                               q3c_coord_t ra2, q3c_coord_t dec2)
{
  /* The FASTEST!!! (and precise) way to compute the distance on the sphere
   * it uses just 3 !!! computation of trigonometric functions
   */
  q3c_coord_t x = q3c_sin ((ra1 - ra2) / 2 * q3c_DEGRA);
  x *= x;
  q3c_coord_t y = q3c_sin ((dec1 - dec2) / 2 * q3c_DEGRA);
  y *= y;

  /* Seem to be more precise :) */
  q3c_coord_t z = q3c_cos ((dec1 + dec2)/2 * q3c_DEGRA);
  z*=z;

  return x * (z - y) + y;
}


void q3c_ang2ipix (struct q3c_prm *hprm, q3c_coord_t ra, q3c_coord_t dec,
                   q3c_ipix_t *ipix)
                   /* ra in degrees, dec in degrees       */
                   /* strictly 0<=ra<360 and -90<=dec<=90 */
{
  q3c_coord_t x0 = 0, y0 = 0, ra1, dec1, tmp0;
  
  q3c_ipix_t nside = hprm->nside, *xbits = hprm->xbits, *ybits = hprm->ybits,
            xi, yi, i1;
  char face_num;

  if (dec == 90)
  /* Poles */
  {
    face_num = 0;
    x0 = q3c_HALF;
    y0 = q3c_HALF;
    goto END1;
  }
  else if (dec == -90)
  {
    face_num = 5;
    x0 = q3c_HALF;
    y0 = q3c_HALF;
    goto END1;
  }

  face_num = q3c_fmod ((ra + 45) / 90, 4); //for equatorial pixels we'll have
                                      //face_num from 1 to 4
  ra1 = q3c_DEGRA * (ra - 90 * (q3c_coord_t)face_num);
  dec1 = q3c_DEGRA * dec;
  x0 = q3c_tan (ra1);
  y0 = q3c_tan (dec1) / q3c_cos (ra1);
  face_num++;
  
  if (y0 > 1) 
  { 
    face_num = 0; 
    ra1 = q3c_DEGRA * ra;
    tmp0 = 1 / q3c_tan (dec1);
#ifdef __USE_GNU
    q3c_sincos (ra1, &x0, &y0);
#else
    x0 = q3c_sin (ra1);
    y0 = q3c_cos (ra1);
#endif
    x0 *= tmp0;
    y0 *= (-tmp0);
//    x0 = q3c_sin(ra1) / q3c_tan(dec1); 
//    y0 = -q3c_cos(ra1) / q3c_tan(dec1);
    /* I don't know 
     * Probably I should write (sin(ra)/sin(dec))*cos(dec) to
     * not loose the precision in the region where dec ~=90deg
     */
  } 
  else 
  {
    if (y0 < -1) 
    { 
      face_num = 5;
      ra1 = q3c_DEGRA * ra;
      tmp0 = 1 / q3c_tan (dec1);
#ifdef __USE_GNU
      q3c_sincos (ra1, &x0, &y0);
#else
      x0 = q3c_sin (ra1);
      y0 = q3c_cos (ra1);
#endif

      x0 *= (-tmp0);
      y0 *= (-tmp0);
//      x0 = -q3c_sin(ra1) / q3c_tan(dec1);
//      y0 = -q3c_cos(ra1) / q3c_tan(dec1);
    }
  }
    
  x0 = (x0 + 1) / 2; y0 = (y0 + 1) / 2;

  END1:
  
  /* Now I produce the final pixel value by converting x and y values to bitfields
    and combining them by interleaving, using the predefined arrays xbits and ybits
  */

  xi = (q3c_ipix_t)(x0 * nside);
  yi = (q3c_ipix_t)(y0 * nside);
  if (xi == nside) xi--; /* This two strings are written to handle the case */
  if (yi == nside) yi--; /* of upper right corner of base square */
  
  i1 = 1 << (q3c_interleaved_nbits);


#ifdef Q3C_INT4 
  {
    *ipix = ((q3c_ipix_t)face_num) * nside * nside +
            xbits[xi % i1] + ybits[yi % i1];
  /*4byte computation*/
  }
#endif /* Q3C_INT4 */
#ifdef Q3C_INT8
  {
    *ipix = ((q3c_ipix_t)face_num) * nside * nside + xbits[xi % i1] +
            ybits[yi % i1] + (xbits[(xi >> q3c_interleaved_nbits) % i1] +
            ybits[(yi >> q3c_interleaved_nbits) % i1]) * i1 * i1;
  /*8byte computation*/
  }
#endif /* Q3C_INT8 */

//  fprintf(stdout,"YYY %d %.20Lf %.20Lf\n",face_num,ra,dec);
//  BIT_PRINT8(*ipix); 
//  BIT_PRINT8ix(xi); 
//  BIT_PRINT8iy(yi);
}



/* Cloned version of ang2ipix for outputting also the x,y on the cube face
 * Coordinates on the cube face are x[-0.5,0.5] y[-0.5,0.5] */
void ang2ipix_xy (struct q3c_prm *hprm, q3c_coord_t ra, q3c_coord_t dec,
                  char *out_face_num, q3c_ipix_t *ipix, q3c_coord_t *x_out, 
                  q3c_coord_t *y_out)
                 /* ra in degrees, dec in degrees       */
                 /* strictly 0<=ra<360 and -90<=dec<=90 */
{
  q3c_coord_t x0 = 0,y0 = 0;
  q3c_ipix_t nside = hprm->nside, *xbits = hprm->xbits, *ybits = hprm->ybits,
             xi, yi, i1;
  char face_num;
  if (dec == 90)
  /* Poles */
  {
    face_num = 0;
    x0 = q3c_HALF;
    y0 = q3c_HALF;
    *x_out = 0;
    *y_out = 0;
    goto END1;
  }
  else if (dec == -90)
  {
    face_num = 5;
    x0 = q3c_HALF;
    y0 = q3c_HALF;
    *x_out = 0;
    *y_out = 0;
    goto END1;
  }
  
  face_num = q3c_fmod ((ra + 45) / 90, 4); 
  /*for equatorial pixels we'll have face_num from 1 to 4 */
  x0 = q3c_tan (q3c_DEGRA * (ra - 90 * (q3c_coord_t)face_num));
  y0 = q3c_tan (dec * q3c_DEGRA) /
       q3c_cos (q3c_DEGRA * (ra - 90 * (q3c_coord_t)face_num));
  face_num++;
  
  if (y0 > 1) 
  { 
    face_num = 0; 
    x0 = q3c_sin (q3c_DEGRA * ra) / q3c_tan (q3c_DEGRA * dec); 
    y0 = -q3c_cos (q3c_DEGRA * ra) / q3c_tan (q3c_DEGRA * dec); 
  } 
  else 
  {
    if (y0 < -1) 
    { 
      face_num = 5;
      x0 = -q3c_sin (q3c_DEGRA * ra) / q3c_tan (q3c_DEGRA * dec);
      y0 = -q3c_cos (q3c_DEGRA * ra) / q3c_tan (q3c_DEGRA * dec);
    }
  }
  
  *x_out = x0 / 2;
  *y_out = y0 / 2;
  x0 = (x0 + 1) / 2;
  y0 = (y0 + 1) / 2;

  END1:
  
  /* Now I produce the final pixel value by converting x and y values to bitfields
   * and combining them by interleaving, using the predefined arrays xbits and ybits
   */

  xi = (q3c_ipix_t)(x0 * nside);
  yi = (q3c_ipix_t)(y0 * nside);
  
  /* This two strings are written to handle the case of edges of base square */
  if (xi == nside) 
  {
    xi--; 
  }
  if (yi == nside)
  {
    yi--; 
  }
  
  i1 = 1 << (q3c_interleaved_nbits);


#ifdef Q3C_INT4 
  {
    *ipix = ((q3c_ipix_t)face_num) * nside * nside + xbits[xi % i1] +
            ybits[yi % i1];
  /*4byte computation*/
  }
#endif /* Q3C_INT4 */
#ifdef Q3C_INT8
  {
    *ipix = ((q3c_ipix_t)face_num) * nside * nside + xbits[xi % i1] +
            ybits[yi % i1] + (xbits[(xi >> q3c_interleaved_nbits) % i1] +
            ybits[(yi >> q3c_interleaved_nbits) % i1]) * i1 * i1;
  /*8byte computation*/
  }
#endif /* Q3C_INT8 */

  *out_face_num = face_num;
//  fprintf(stdout,"YYY %d %.20Lf %.20Lf\n",face_num,ra,dec);
//  BIT_PRINT8(*ipix); 
//  BIT_PRINT8ix(xi); 
//  BIT_PRINT8iy(yi);
}


/* The code extracted from ang2ipix for getting the cube face number
 */
char q3c_getfacenum(q3c_coord_t ra, q3c_coord_t dec)
                    /* ra in degrees, dec in degrees       */
                    /* strictly 0<=ra<360 and -90<=dec<=90 */
{
  q3c_coord_t y0 = 0;
  char face_num;
  
  if (dec == 90)
  /* Poles */
  {
    return 0;
  }
  else if (dec == -90)
  {
    return 5;
  }
  face_num = q3c_fmod ((ra + 45) / 90, 4); //for equatorial pixels we'll have
                                      //face_num from 1 to 4
  y0 = q3c_tan(dec * q3c_DEGRA) /
                    q3c_cos(q3c_DEGRA * (ra - 90 * (q3c_coord_t)face_num));
  face_num++;
  
  if (y0 > 1) 
  { 
    return 0; 
  } 
  else if (y0 < -1) 
  { 
      return 5;
  }
  else 
  {
    return face_num;
  }
}


void q3c_get_nearby_split (struct q3c_prm *hprm, q3c_coord_t ra,
                           q3c_coord_t dec, q3c_coord_t radius,
                           q3c_ipix_t *ipix, int iteration)
                          /* ra in degrees, dec in degrees,
                           * radius in degrees
                           * strictly 0<=ra<360 and -90<=dec<=90
                           */
{
  q3c_coord_t xmin, xmax, ymin, ymax, xesize, yesize, points[4];
  q3c_ipix_t nside = hprm->nside, *xbits = hprm->xbits, *ybits = hprm->ybits,
            *ipix_cur = ipix, ipix0, xi = 0, yi = 0, i1, n0, n1 = 1, ixmin,
            ixmax, iymin, iymax;
  /* I did the initialization of xi, yi and n1 just to prevent the gcc
   * warnings 
   */
  char face_num, multi_flag;
  
  const q3c_coord_t q3c_lg2 = q3c_LG2;
  
  face_num = q3c_getfacenum(ra, dec);

  /*
  get_poly_coefs(face_num, ra, dec, radius, &axx, &ayy, &axy, &ax, &ay, &a);
  get_xy_minmax(axx, ayy, axy, ax, ay, a, &xmin, &xmax, &ymin, &ymax);
  */
  q3c_fast_get_circle_xy_minmax(face_num, ra, dec, radius, &xmin, &xmax, &ymin, &ymax);
  /* xmin, xmax, ymin, ymax are in the coordinate system of the cube face 
   * where -0.5<=x<=0.5 and -0.5<=y<=0.5 
   */
#ifdef Q3C_DEBUG
fprintf(stderr, "XMIN: %f XMAX: %f YMIN: %f YMAX: %f\n", xmin, xmax, ymin, ymax);
#endif
  

  /* Now in a little bit ugly but fastest way I determine whether the ellipse 
   * intersect other faces or not, and if yes, I setup the array "points" to the
   * multi_face loop.
   * The multi_flag value specify how much additional faces are covered by the 
   * query area.
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
          multi_flag = 1;
          ymax = q3c_HALF;
        }
        else
        {
          multi_flag = 0;
        }
      }
    }
  }
  
  
  if (multi_flag == 0)
  {    
    xesize = xmax - xmin; 
    yesize = ymax - ymin;
    xesize = xesize > yesize ? xesize : yesize;
    
    if (xesize * nside < 1) 
    /* If the region is too small */
    {
      xesize = 1 / (q3c_coord_t)nside;
    }
    
    n0 = 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(xesize) / q3c_lg2)));  
    /* n0 is now the level of quadtree for which the minimal 
     * element is >~ our ellipse
     */

    ixmin = (q3c_HALF + xmin) * n0;
    ixmax = (q3c_HALF + xmax) * n0;
    iymin = (q3c_HALF + ymin) * n0;
    iymax = (q3c_HALF + ymax) * n0;
    
    ixmax = (ixmax == n0 ? n0-1 : ixmax);
    iymax = (iymax == n0 ? n0-1 : iymax);
    
    n1 = nside / n0;

    if (iymin == iymax) 
    {
      if (iteration > 2) 
      {
        *(ipix_cur++) = 1;
        *(ipix_cur) = -1;      
        return;
      }
      if (ixmin == ixmax)
      {
        if (iteration > 1)
        {
          *(ipix_cur++) = 1;
          *(ipix_cur) = -1;      
          return;
        }
      }
      if (iteration == 1)
      {
        xi = (q3c_ipix_t)(ixmin * n1);
        yi = (q3c_ipix_t)(iymin * n1);
      }
      else 
      {
        xi = (q3c_ipix_t)(ixmax * n1);
        yi = (q3c_ipix_t)(iymin * n1);
      }
    }
    else
    {
      if (ixmin == ixmax)
      {
        if (iteration > 2)
        {
          *(ipix_cur++) = 1;
          *(ipix_cur) = -1;      
          return;
        }

      }
      switch(iteration) 
      {
        case 1: 
        {
          xi = (q3c_ipix_t)(ixmin * n1);
          yi = (q3c_ipix_t)(iymin * n1);
          break;
        }
        case 2:
        {
          xi = (q3c_ipix_t)(ixmin * n1);
          yi = (q3c_ipix_t)(iymax * n1);
          break;
        }
        case 3: 
        {
          xi = (q3c_ipix_t)(ixmax * n1);
          yi = (q3c_ipix_t)(iymin * n1);
          break;
        }
        case 4: 
        {
          xi = (q3c_ipix_t)(ixmax * n1);
          yi = (q3c_ipix_t)(iymax * n1);
          break;
        }
      }
    }
  }
  else 
  {
    if (multi_flag == 1) 
    {
      if (iteration <= 2)
      {
        xesize = xmax - xmin; 
        yesize = ymax - ymin;
        xesize = xesize > yesize ? xesize : yesize;
        
        if (xesize * nside < 1) 
        /* If the region is too small */
        {
          xesize = 1 / (q3c_coord_t)nside;
        }
        
        n0 = 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(xesize)/q3c_lg2)));  
        /* n0 is now the level of quadtree for which the minimal 
         * element is >~ our ellipse
         */

        ixmin = (q3c_HALF + xmin) * n0;
        ixmax = (q3c_HALF + xmax) * n0;
        iymin = (q3c_HALF + ymin) * n0;
        iymax = (q3c_HALF + ymax) * n0;
        
        ixmax = (ixmax == n0 ? n0-1 : ixmax);
        iymax = (iymax == n0 ? n0-1 : iymax);
        
        n1 = nside / n0;
        
        if (ixmin == ixmax)
        {
          if (iteration == 1)
          {
            xi = (q3c_ipix_t)(ixmax * n1);
            yi = (q3c_ipix_t)(iymin * n1);
          }
          else 
          {
            if (iymin == iymax)
            {
              *(ipix_cur++) = 1;
              *(ipix_cur) = -1;      
              return;
            }
            else 
            {
              xi = (q3c_ipix_t)(ixmax * n1);
              yi = (q3c_ipix_t)(iymax * n1);
            }
          }
        } 
        else
        {
          if (iteration == 1)
          {
            xi = (q3c_ipix_t)(ixmin * n1);
            yi = (q3c_ipix_t)(iymax * n1);
          }
          else 
          {
            xi = (q3c_ipix_t)(ixmax * n1);
            yi = (q3c_ipix_t)(iymax * n1);
          }
        }
      }
      else 
      {
        face_num = q3c_xy2facenum(2 * points[0], 2 * points[1], face_num);
        /*
        get_poly_coefs(face_num, ra, dec, radius, &axx, &ayy, &axy, &ax, &ay, &a);
        get_xy_minmax(axx, ayy, axy, ax, ay, a, &xmin, &xmax, &ymin, &ymax);
        */
        q3c_fast_get_circle_xy_minmax(face_num,ra,dec,radius,&xmin,&xmax,&ymin,&ymax);
        
        xmax = (xmax > q3c_HALF ? q3c_HALF : xmax);
        xmin = (xmin < -q3c_HALF ? -q3c_HALF : xmin);
        ymax = (ymax > q3c_HALF ? q3c_HALF : ymax);
        ymin = (ymin < -q3c_HALF ? -q3c_HALF : ymin);
        xesize = xmax - xmin; 
        yesize = ymax - ymin;
        xesize = xesize > yesize ? xesize : yesize;
        
        if (xesize * nside < 1) 
        /* If the region is too small */
        {
          xesize = 1 / (q3c_coord_t)nside;
        }
        
        n0 = 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(xesize)/q3c_lg2)));  
        /* n0 is now the level of quadtree for which the minimal 
         * element is >~ our ellipse
         */

        ixmin = (q3c_HALF + xmin) * n0;
        ixmax = (q3c_HALF + xmax) * n0;
        iymin = (q3c_HALF + ymin) * n0;
        iymax = (q3c_HALF + ymax) * n0;
        
        ixmax = (ixmax == n0 ? n0-1 : ixmax);
        iymax = (iymax == n0 ? n0-1 : iymax);
        
        n1 = nside / n0;        

        if (ixmin == ixmax)
        {
          if (iteration == 3)
          {
            xi = (q3c_ipix_t)(ixmax * n1);
            yi = (q3c_ipix_t)(iymin * n1);
          }
          else 
          {
            if (iymin == iymax)
            {
              *(ipix_cur++) = 1;
              *(ipix_cur) = -1;      
              return;
            }
            else 
            {
              xi = (q3c_ipix_t)(ixmax * n1);
              yi = (q3c_ipix_t)(iymax * n1);
            }
          }
        } 
        else
        {
          if (iteration == 3)
          {
            xi = (q3c_ipix_t)(ixmin * n1);
            yi = (q3c_ipix_t)(iymax * n1);
          }
          else 
          {
            xi = (q3c_ipix_t)(ixmax * n1);
            yi = (q3c_ipix_t)(iymax * n1);
          }
        }
      }
    }
    else 
    {
      switch(iteration)
      {
        case 1:
        {
          xesize = xmax - xmin; 
          yesize = ymax - ymin;
          xesize = xesize > yesize ? xesize : yesize;
          
          if (xesize * nside < 1) 
          /* If the region is too small */
          {
            xesize = 1 / (q3c_coord_t)nside;
          }
          
          n0 = 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(xesize) / q3c_lg2)));  
          /* n0 is now the level of quadtree for which the minimal 
           * element is >~ our ellipse
           */

          ixmin = (q3c_HALF + xmin) * n0;
          ixmax = (q3c_HALF + xmax) * n0;
          iymin = (q3c_HALF + ymin) * n0;
          iymax = (q3c_HALF + ymax) * n0;
          
          ixmax = (ixmax == n0 ? (n0 - 1) : ixmax);
          iymax = (iymax == n0 ? (n0 - 1) : iymax);

          n1 = nside / n0;
          
          xi=(q3c_ipix_t)(ixmax * n1);
          yi=(q3c_ipix_t)(iymax * n1);
          break;
        }
        case 2:
        {
          face_num = q3c_xy2facenum(2 * points[0], 2 * points[1], face_num);
          /*
          get_poly_coefs(face_num, ra, dec, radius, &axx, &ayy, &axy, &ax, &ay, &a);
          get_xy_minmax(axx, ayy, axy, ax, ay, a, &xmin, &xmax, &ymin, &ymax);
          */
          q3c_fast_get_circle_xy_minmax(face_num, ra, dec, radius, &xmin, &xmax,
                                 &ymin, &ymax);
          
          xmax = (xmax > q3c_HALF ? q3c_HALF : xmax);
          xmin = (xmin < -q3c_HALF ? -q3c_HALF : xmin);
          ymax = (ymax > q3c_HALF ? q3c_HALF : ymax);
          ymin = (ymin < -q3c_HALF ? -q3c_HALF : ymin);
          xesize = xmax - xmin; 
          yesize = ymax - ymin;
          xesize = xesize > yesize ? xesize : yesize;
          
          if (xesize*nside<1) 
          /* If the region is too small */
          {
            xesize=1 / (q3c_coord_t)nside;
          }
          
          n0 = 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(xesize) / q3c_lg2)));  
          /* n0 is now the level of quadtree for which the minimal 
           * element is >~ our ellipse
           */

          ixmin = (q3c_HALF + xmin) * n0;
          ixmax = (q3c_HALF + xmax) * n0;
          iymin = (q3c_HALF + ymin) * n0;
          iymax = (q3c_HALF + ymax) * n0;
          
          ixmax = (ixmax == n0 ? n0-1 : ixmax);
          iymax = (iymax == n0 ? n0-1 : iymax);
          
          n1 = nside / n0;

          xi = (q3c_ipix_t)(ixmax * n1);
          yi = (q3c_ipix_t)(iymax * n1);
          break;
        }
        case 3:
        {
          face_num = q3c_xy2facenum(2 * points[2], 2 * points[3], face_num);
          /*
          get_poly_coefs(face_num, ra, dec, radius, &axx, &ayy, &axy, &ax, &ay, &a);
          get_xy_minmax(axx, ayy, axy, ax, ay, a, &xmin, &xmax, &ymin, &ymax);
          //*/
          q3c_fast_get_circle_xy_minmax(face_num, ra, dec, radius, &xmin, &xmax,
                                 &ymin, &ymax);
          
          xmax = (xmax > q3c_HALF ? q3c_HALF : xmax);
          xmin = (xmin < -q3c_HALF ? -q3c_HALF : xmin);
          ymax = (ymax > q3c_HALF ? q3c_HALF : ymax);
          ymin = (ymin < -q3c_HALF ? -q3c_HALF : ymin);
          xesize = xmax - xmin; 
          yesize = ymax - ymin;
          xesize = xesize > yesize ? xesize : yesize;
          
          if (xesize * nside < 1) 
          /* If the region is too small */
          {
            xesize=1 / (q3c_coord_t)nside;
          }
          
          n0 = 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(xesize) / q3c_lg2)));  
          /* n0 is now the level of quadtree for which the minimal 
           * element is >~ our ellipse
           */

          ixmin = (q3c_HALF + xmin) * n0;
          ixmax = (q3c_HALF + xmax) * n0;
          iymin = (q3c_HALF + ymin) * n0;
          iymax = (q3c_HALF + ymax) * n0;
          
          ixmax = (ixmax == n0 ? n0-1 : ixmax);
          iymax = (iymax == n0 ? n0-1 : iymax);
          
          n1 = nside / n0;

          xi = (q3c_ipix_t)(ixmax * n1);
          yi = (q3c_ipix_t)(iymax * n1);
          break;
        }
        case 4:
        {
          *(ipix_cur++) = 1;
          *(ipix_cur) = -1;      
          return;
        }
      }
    }
  }

  
  /* Now I produce the final pixel value by converting x and y values to bitfields
   * and combining them by interleaving, using the predefined arrays xbits and ybits
   */


  i1=1 << (q3c_interleaved_nbits);
  
#ifdef Q3C_INT4 
  {
    ipix0 = ((q3c_ipix_t)face_num) * nside * nside + 
              xbits[xi % i1] + ybits[yi % i1];
  /*4byte computation*/
  }
#endif /* Q3C_INT4 */
#ifdef Q3C_INT8
  {
    ipix0 = ((q3c_ipix_t)face_num) * nside * nside + 
              xbits[xi % i1] + ybits[yi % i1] +
    (xbits[(xi >> q3c_interleaved_nbits) % i1] + 
     ybits[(yi >> q3c_interleaved_nbits) % i1]) * i1 * i1;
  /*8byte computation*/
  }
#endif /* Q3C_INT8 */

  *(ipix_cur++) = ipix0;
  *ipix_cur = ipix0 + n1 * n1 - 1;
  /* IMPORTANT!! I subtract 1 to make after the query with   <= ipix<=
   */
  
//  fprintf(stdout,"YYY %d %.20Lf %.20Lf\n",face_num,ra,dec);
//  BIT_PRINT8(*ipix); 
//  BIT_PRINT8ix(xi); 
//  BIT_PRINT8iy(yi);
}




void q3c_get_nearby(struct q3c_prm *hprm, q3c_coord_t ra, q3c_coord_t dec,
                q3c_coord_t radius, q3c_ipix_t *ipix)
                /* ra in degrees, dec in degrees, radius in degrees */
                /* strictly 0<=ra<360 and -90<=dec<=90 */
{
  q3c_coord_t xmin, xmax, ymin, ymax, xesize, yesize, points[4];
  q3c_ipix_t nside = hprm->nside, *xbits = hprm->xbits, *ybits = hprm->ybits,
             *ipix_cur = ipix, ipix0, xi, yi, i1, n0, n1, ixmin,
             ixmax, iymin, iymax, xistack[4], yistack[4], facestack[4],
             nstack[4];
  char face_num, face_num0, multi_flag;
  int i, nistack = 0;
  const q3c_coord_t q3c_lg2 = q3c_LG2;

  face_num = q3c_getfacenum(ra, dec);
  face_num0 = face_num;
  
  q3c_fast_get_circle_xy_minmax(face_num, ra, dec, radius, &xmin, &xmax, &ymin, &ymax);
  /* xmin, xmax, ymin, ymax are in the coordinate system of the cube face 
   * where -0.5<=x<=0.5 and -0.5<=y<=0.5 
   */
#ifdef Q3C_DEBUG
  fprintf(stderr, "XMIN: %f XMAX: %f YMIN: %f YMAX: %f\n", xmin, xmax, ymin, ymax);
#endif

  
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
          multi_flag = 1;
          ymax = q3c_HALF;
        }
        else
        {
          multi_flag = 0;
        }
      }
    }
  }
  
  
  if (multi_flag == 0)
  {    
    xesize = xmax - xmin; 
    yesize = ymax - ymin;
    xesize = xesize > yesize ? xesize : yesize;
    
    if (xesize * nside < 1) 
    /* If the region is too small */
    {
      xesize = 1 / (q3c_coord_t)nside;
    }
    
    n0 = 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(xesize) / q3c_lg2)));  
    /* n0 is now the level of quadtree for which the minimal 
     * element is >~ our ellipse
     */

    ixmin = (q3c_HALF + xmin) * n0;
    ixmax = (q3c_HALF + xmax) * n0;
    iymin = (q3c_HALF + ymin) * n0;
    iymax = (q3c_HALF + ymax) * n0;
    
    ixmax = (ixmax == n0 ? n0 - 1 : ixmax);
    iymax = (iymax == n0 ? n0 - 1 : iymax);
    
    n1 = nside / n0;

    if (iymin == iymax) 
    {
      if (ixmin == ixmax)
      {
        xistack[0] = (q3c_ipix_t)(ixmin * n1);
        yistack[0] = (q3c_ipix_t)(iymin * n1);
        facestack[0] = face_num;
        nstack[0] = n1;
        nistack = 1;
      }
      else
      {
        xistack[0] = (q3c_ipix_t)(ixmin * n1);
        yistack[0] = (q3c_ipix_t)(iymin * n1);
        facestack[0] = face_num;
        nstack[0] = n1;
        xistack[1] = (q3c_ipix_t)(ixmax * n1);
        yistack[1] = (q3c_ipix_t)(iymin * n1);
        facestack[1] = face_num;
        nstack[1] = n1;
        nistack = 2;      
      }
    }
    else
    {
      if (ixmin == ixmax)
      {
        xistack[0] = (q3c_ipix_t)(ixmin * n1);
        yistack[0] = (q3c_ipix_t)(iymin * n1);
        facestack[0] = face_num;
        nstack[0] = n1;
        xistack[1] = (q3c_ipix_t)(ixmin * n1);
        yistack[1] = (q3c_ipix_t)(iymax * n1);
        facestack[1] = face_num;
        nstack[1] = n1;
        nistack = 2;      
      }
      else 
      {
        xistack[0] = (q3c_ipix_t)(ixmin * n1);
        yistack[0] = (q3c_ipix_t)(iymin * n1);
        facestack[0] = face_num;
        nstack[0] = n1;
        xistack[1] = (q3c_ipix_t)(ixmin * n1);
        yistack[1] = (q3c_ipix_t)(iymax * n1);
        facestack[1] = face_num;
        nstack[1] = n1;
        xistack[2] = (q3c_ipix_t)(ixmax * n1);
        yistack[2] = (q3c_ipix_t)(iymin * n1);
        facestack[2] = face_num;
        nstack[2] = n1;
        xistack[3] = (q3c_ipix_t)(ixmax * n1);
        yistack[3] = (q3c_ipix_t)(iymax * n1);
        facestack[3] = face_num;
        nstack[3] = n1;
        nistack = 4;            
      }
    }    
  }
  else 
  {
    if (multi_flag == 1) 
    {
      xesize = xmax - xmin; 
      yesize = ymax - ymin;
      xesize = xesize > yesize ? xesize : yesize;
      
      if (xesize * nside < 1) 
      /* If the region is too small */
      {
        xesize = 1 / (q3c_coord_t)nside;
      }
      
      n0 = 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(xesize) / q3c_lg2)));  
      /* n0 is now the level of quadtree for which the minimal 
       * element is >~ our ellipse
       */

      ixmin = (q3c_HALF + xmin) * n0;
      ixmax = (q3c_HALF + xmax) * n0;
      iymin = (q3c_HALF + ymin) * n0;
      iymax = (q3c_HALF + ymax) * n0;
      
      ixmax = (ixmax == n0 ? n0 - 1 : ixmax);
      iymax = (iymax == n0 ? n0 - 1 : iymax);
      
      n1 = nside / n0;
      
      if (ixmin == ixmax)
      {
        if (iymin == iymax)
        {
          xistack[0] = (q3c_ipix_t)(ixmin * n1);
          yistack[0] = (q3c_ipix_t)(iymin * n1);
          facestack[0] = face_num;
          nstack[0] = n1;
          nistack = 1;
        }
        else 
        {
          xistack[0] = (q3c_ipix_t)(ixmin * n1);
          yistack[0] = (q3c_ipix_t)(iymin * n1);
          facestack[0] = face_num;
          nstack[0] = n1;
          xistack[1] = (q3c_ipix_t)(ixmin * n1);
          yistack[1] = (q3c_ipix_t)(iymax * n1);
          facestack[1] = face_num;
          nstack[0] = n1;
          nistack = 2;
        }
      }
      else 
      {
        xistack[0] = (q3c_ipix_t)(ixmin * n1);
        yistack[0] = (q3c_ipix_t)(iymin * n1);
        facestack[0] = face_num;
        nstack[0] = n1;
        xistack[1] = (q3c_ipix_t)(ixmax * n1);
        yistack[1] = (q3c_ipix_t)(iymin * n1);
        facestack[1] = face_num;
        nstack[1] = n1;
        nistack = 2;        
      }

      face_num = q3c_xy2facenum(2 * points[0], 2 * points[1], face_num0);
      q3c_fast_get_circle_xy_minmax(face_num, ra, dec, radius, &xmin, &xmax, &ymin,
                             &ymax);
      
      xmax = (xmax > q3c_HALF ? q3c_HALF : xmax);
      xmin = (xmin < -q3c_HALF ? -q3c_HALF : xmin);
      ymax = (ymax > q3c_HALF ? q3c_HALF : ymax);
      ymin = (ymin < -q3c_HALF ? -q3c_HALF : ymin);
      xesize = xmax - xmin; 
      yesize = ymax - ymin;
      xesize = xesize > yesize ? xesize : yesize;
      
      if (xesize * nside < 1) 
      /* If the region is too small */
      {
        xesize = 1 / (q3c_coord_t)nside;
      }
      
      n0 = 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(xesize) / q3c_lg2)));  
      /* n0 is now the level of quadtree for which the minimal 
       * element is >~ our ellipse
       */

      ixmin = (q3c_HALF + xmin) * n0;
      ixmax = (q3c_HALF + xmax) * n0;
      iymin = (q3c_HALF + ymin) * n0;
      iymax = (q3c_HALF + ymax) * n0;
      
      ixmax = (ixmax == n0 ? n0 - 1 : ixmax);
      iymax = (iymax == n0 ? n0 - 1 : iymax);
      
      n1 = nside / n0;        

      if (ixmin == ixmax)
      {
        if (iymin == iymax)
        {
          xi = (q3c_ipix_t)(ixmax * n1);
          yi = (q3c_ipix_t)(iymin * n1);
          xistack[nistack] = (q3c_ipix_t)(ixmin * n1);
          yistack[nistack] = (q3c_ipix_t)(iymin * n1);
          facestack[nistack] = face_num;
          nstack[nistack++] = n1;
        }
        else 
        {
          xistack[nistack] = (q3c_ipix_t)(ixmin * n1);
          yistack[nistack] = (q3c_ipix_t)(iymin * n1);
          facestack[nistack] = face_num;
          nstack[nistack++] = n1;
          xistack[nistack] = (q3c_ipix_t)(ixmin * n1);
          yistack[nistack] = (q3c_ipix_t)(iymax * n1);
          facestack[nistack] = face_num;
          nstack[nistack++] = n1;
        }
      }
      else 
      {
        xistack[nistack] = (q3c_ipix_t)(ixmin * n1);
        yistack[nistack] = (q3c_ipix_t)(iymin * n1);
        facestack[nistack] = face_num;
        nstack[nistack++] = n1;
        xistack[nistack] = (q3c_ipix_t)(ixmax * n1);
        yistack[nistack] = (q3c_ipix_t)(iymin * n1);
        facestack[nistack] = face_num;
        nstack[nistack++] = n1;
      }
    }
    else 
    {
      xesize = xmax - xmin; 
      yesize = ymax - ymin;
      xesize = xesize > yesize ? xesize : yesize;
      
      if (xesize * nside < 1) 
      /* If the region is too small */
      {
        xesize=1 / (q3c_coord_t)nside;
      }
      
      n0 = 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(xesize) / q3c_lg2)));  
      /* n0 is now the level of quadtree for which the minimal 
       * element is >~ our ellipse
       */

      ixmin = (q3c_HALF + xmin) * n0;
      ixmax = (q3c_HALF + xmax) * n0;
      iymin = (q3c_HALF + ymin) * n0;
      iymax = (q3c_HALF + ymax) * n0;
      
      ixmax = (ixmax == n0 ? n0 - 1 : ixmax);
      iymax = (iymax == n0 ? n0 - 1 : iymax);

      n1 = nside / n0;
      
      xistack[0] = (q3c_ipix_t)(ixmin * n1);
      yistack[0] = (q3c_ipix_t)(iymin * n1);
      facestack[0] = face_num;
      nstack[0] = n1;
      nistack = 1;

      face_num = q3c_xy2facenum(2 * points[0], 2 * points[1], face_num0);
      q3c_fast_get_circle_xy_minmax(face_num, ra, dec, radius, &xmin, &xmax, &ymin,
                             &ymax);
      
      xmax = (xmax > q3c_HALF ? q3c_HALF : xmax);
      xmin = (xmin < -q3c_HALF ? -q3c_HALF : xmin);
      ymax = (ymax > q3c_HALF ? q3c_HALF : ymax);
      ymin = (ymin < -q3c_HALF ? -q3c_HALF : ymin);
      xesize = xmax - xmin; 
      yesize = ymax - ymin;
      xesize = xesize > yesize ? xesize : yesize;
      
      if (xesize * nside < 1) 
      /* If the region is too small */
      {
        xesize=1 / (q3c_coord_t)nside;
      }
      
      n0 = 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(xesize) / q3c_lg2)));  
      /* n0 is now the level of quadtree for which the minimal 
       * element is >~ our ellipse
       */

      ixmin = (q3c_HALF + xmin) * n0;
      ixmax = (q3c_HALF + xmax) * n0;
      iymin = (q3c_HALF + ymin) * n0;
      iymax = (q3c_HALF + ymax) * n0;
      
      ixmax = (ixmax == n0 ? n0 - 1 : ixmax);
      iymax = (iymax == n0 ? n0 - 1 : iymax);
      
      n1 = nside / n0;

      xistack[1] = (q3c_ipix_t)(ixmin * n1);
      yistack[1] = (q3c_ipix_t)(iymin * n1);
      facestack[1] = face_num;
      nstack[1] = n1;
      nistack = 2;


      face_num = q3c_xy2facenum(2 * points[2], 2 * points[3], face_num0);
      q3c_fast_get_circle_xy_minmax(face_num, ra, dec, radius, &xmin, &xmax, &ymin,
                             &ymax);
      
      xmax = (xmax > q3c_HALF ? q3c_HALF : xmax);
      xmin = (xmin < -q3c_HALF ? -q3c_HALF : xmin);
      ymax = (ymax > q3c_HALF ? q3c_HALF : ymax);
      ymin = (ymin < -q3c_HALF ? -q3c_HALF : ymin);
      xesize = xmax - xmin; 
      yesize = ymax - ymin;
      xesize = xesize > yesize ? xesize : yesize;
      
      if (xesize * nside < 1) 
      /* If the region is too small */
      {
        xesize = 1 / (q3c_coord_t)nside;
      }
      
      n0 = 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(xesize) / q3c_lg2)));  
      /* n0 is now the level of quadtree for which the minimal 
       * element is >~ our ellipse
       */

      ixmin = (q3c_HALF + xmin) * n0;
      ixmax = (q3c_HALF + xmax) * n0;
      iymin = (q3c_HALF + ymin) * n0;
      iymax = (q3c_HALF + ymax) * n0;
      
      ixmax = (ixmax == n0 ? n0-1 : ixmax);
      iymax = (iymax == n0 ? n0-1 : iymax);
      
      n1 = nside / n0;

      xistack[2] = (q3c_ipix_t)(ixmin * n1);
      yistack[2] = (q3c_ipix_t)(iymin * n1);
      facestack[2] = face_num;
      nstack[2] = n1;
      nistack = 3;
    }
  }

  
  /* Now I produce the final pixel value by converting x and y values to bitfields
    and combining them by interleaving, using the predefined arrays xbits and ybits
  */

  i1=1 << (q3c_interleaved_nbits);

  for(i = 0; i < nistack ; i++)
  {
    face_num = facestack[i];
    xi = xistack[i];
    yi = yistack[i];
    n1 = nstack[i];    
    
#ifdef Q3C_INT4 
    {
      ipix0 = ((q3c_ipix_t)face_num) * nside * nside + 
                xbits[xi % i1] + ybits[yi % i1];
    /*4byte computation*/
    }
#endif /* Q3C_INT4 */
#ifdef Q3C_INT8
    {
      ipix0 = ((q3c_ipix_t)face_num) * nside * nside + 
                xbits[xi % i1] + ybits[yi % i1] +
      (xbits[(xi >> q3c_interleaved_nbits) % i1] + 
       ybits[(yi >> q3c_interleaved_nbits) % i1]) * i1 * i1;
    /*8byte computation*/
    }
#endif /* Q3C_INT8 */
    *(ipix_cur++) = ipix0;
    *(ipix_cur++) = ipix0 + n1 * n1 - 1;
    /* IMPORTANT!! I subtract 1 to make after the query with   <= ipix<=
     */
  }

  for(; i < 4; i++)
  {
    *(ipix_cur++) = 1;
    *(ipix_cur++) = -1;
  }

}


inline q3c_ipix_t q3c_xiyi2ipix(q3c_ipix_t nside, q3c_ipix_t *xbits,
                                q3c_ipix_t *ybits, char face_num, 
                                q3c_ipix_t xi, q3c_ipix_t yi)
{
  const q3c_ipix_t  i1=1 << (q3c_interleaved_nbits);
  
#ifdef Q3C_INT4 
  {
    return ((q3c_ipix_t)face_num) * nside * nside + 
              xbits[xi % i1] + ybits[yi % i1];
  /*4byte computation*/
  }
#endif /* Q3C_INT4 */
#ifdef Q3C_INT8
  {
    return ((q3c_ipix_t)face_num) * nside * nside + 
              xbits[xi % i1] + ybits[yi % i1] +
    (xbits[(xi >> q3c_interleaved_nbits) % i1] + 
     ybits[(yi >> q3c_interleaved_nbits) % i1]) * i1 * i1;
  /*8byte computation*/
  }
#endif /* Q3C_INT8 */
  
}


void q3c_ipix2ang(struct q3c_prm *hprm, q3c_ipix_t ipix, 
              q3c_coord_t *ra, q3c_coord_t *dec)
{
  q3c_ipix_t nside=hprm->nside, ipix1, *xbits1=hprm->xbits1, 
              *ybits1=hprm->ybits1, i2, i3, x0, y0;
  
  q3c_coord_t x,y,ra0;
  char face_num = ipix / (nside * nside);
  const q3c_ipix_t i1=1<<q3c_interleaved_nbits;
  const q3c_ipix_t ii1=1<<(q3c_interleaved_nbits / 2);
  ipix1 = ipix % (nside * nside);
  
#ifdef Q3C_INT4   
  i3 = ipix1 % i1; i2 = ipix1 / i1;
  x0 = xbits1[i3]; y0 = ybits1[i3];
  i3 = i2 % i1; i2 = i2 / i1;
  x0 += xbits1[i3] * ii1; y0 += ybits1[i3] * ii1;
#endif /* Q3C_INT4 */

#ifdef Q3C_INT8
//  fprintf(stdout,"__\n");
  i3 = ipix1 % i1; i2 = ipix1 / i1;
  x0 = xbits1[i3]; y0 = ybits1[i3];
  i3 = i2 % i1; i2 = i2 / i1;
  x0 += xbits1[i3] * ii1; y0 += ybits1[i3] * ii1;
  i3 = i2 % i1; i2 = i2 / i1;
  x0 += xbits1[i3] * ii1 * ii1; y0 += ybits1[i3] * ii1 * ii1;  
  i3 = i2 % i1; i2 = i2 / i1;
  x0 += xbits1[i3] * ii1 * ii1 * ii1; y0 += ybits1[i3] * ii1 * ii1 * ii1;
//  BIT_PRINT8(ipix);
//  BIT_PRINT8ix(x0);
//  BIT_PRINT8iy(y0);
#endif /* Q3C_INT8 */  
  
  x = (((q3c_coord_t)x0) / nside) * 2 - 1;
  y=(((q3c_coord_t)y0) / nside) * 2 - 1;
  /* Now -1<x<1 and -1<y<1 */

//  fprintf(stdout,"%Lf %Lf %Lf %Lf\n",(x+1)/2,(y+1)/2,x,y);
  
  
  if ((face_num >= 1) && (face_num <= 4))
  {
    ra0 = q3c_atan(x);
    *dec = q3c_RADEG * q3c_atan(y * q3c_cos(ra0));
    ra0 = ra0 * q3c_RADEG + ((q3c_coord_t)face_num - 1) * 90;
    if (ra0 < 0) 
    {
      ra0 += (q3c_coord_t)360;
    }
    *ra = ra0;
  }
  else 
  {
    if (face_num == 0)
    {
      ra0 = q3c_RADEG * (q3c_atan2(-x, y) + q3c_PI);
      *dec = q3c_RADEG * q3c_atan( 1 / q3c_sqrt(x * x + y * y));
      *ra = ra0;
    }
    if (face_num == 5)
    {
      ra0 = q3c_RADEG * (q3c_atan2(-x, -y) + q3c_PI);
      *dec = -q3c_RADEG * q3c_atan(1 / q3c_sqrt(x * x + y * y));
      *ra = ra0;
    }    
  }  
//  fprintf(stdout,"XXX %d %.20Lf %.20Lf\n",face_num,*ra,*dec);
}

char q3c_xy2facenum(q3c_coord_t x, q3c_coord_t y, char face_num0)
{
  /* The input x, y should be >=-1  and <=1 */
  
  
  q3c_coord_t ra = 0, dec = 0; 
  /* I do the initialization since gcc warn about probable not 
   * initialization of ra and dec 
   */
  
  /* This code have been cutted out from ipix2ang BEGIN */
  if ((face_num0 >= 1) && (face_num0 <= 4))
  {
    ra = q3c_atan(x);
    dec = q3c_RADEG * q3c_atan(y * q3c_cos(ra));
    ra = ra * q3c_RADEG + ((q3c_coord_t)face_num0 - 1) * 90;
    if (ra < 0) 
    {
      ra += (q3c_coord_t)360;
    }
  }
  else 
  {
    if (face_num0 == 0)
    {
      ra = q3c_RADEG * q3c_atan2(x, -y);
      dec = q3c_RADEG * q3c_atan(1 / q3c_sqrt(x * x + y * y));
      if (ra < 0) 
      {
        ra += (q3c_coord_t)360;
      }
    }
    if (face_num0 == 5)
    {
      ra = q3c_RADEG * q3c_atan2(x, y);
      dec = -q3c_RADEG * q3c_atan(1 / q3c_sqrt(x * x + y * y));
      if (ra < 0) 
      {
        ra += (q3c_coord_t)360;
      }

    }
  }
  /* This code have been cutted out from ipix2ang END */

  return q3c_getfacenum(ra,dec)  ;  
}




/* Initialization of the Q3CUBE structure */
void init_q3c1(struct q3c_prm *hprm, q3c_ipix_t nside)
/* hprm -- Pointer to main Q3C structure 
 * nside -- Nside parameter (number of quadtree subdivisions) 
 */
{
  int i, k, m, l;
  const q3c_ipix_t nbits = q3c_interleaved_nbits;
                      /* Number of bits used when interleaving bits
                       * so the size of each allocated array will be 2^16 */
  q3c_ipix_t *xbits, *ybits, *xbits1, *ybits1, xybits_size = 1 << nbits;
  hprm->nside = nside;
  xbits = malloc((xybits_size) * sizeof(q3c_ipix_t));
  hprm->xbits = xbits;
  ybits = malloc((xybits_size) * sizeof(q3c_ipix_t));
  hprm->ybits = ybits;
  xbits1 = malloc((xybits_size) * sizeof(q3c_ipix_t));
  hprm->xbits1 = xbits1;
  ybits1 = malloc((xybits_size) * sizeof(q3c_ipix_t));
  hprm->ybits1 = ybits1;

  xbits[0] = 0; xbits[1] = 1;
  ybits[0] = 0; ybits[1] = 2;
  /*BIT_PRINT8(xbits[0]);
  BIT_PRINT8(xbits[1]);*/
  for(i = 2, m = 1; i < xybits_size; i++)
  {
    k = i / m;
    if (k == 2)
    {
      xbits[i] = xbits[i / 2] * 4;
      ybits[i] = 2 * xbits[i];    
      m *= 2;
      //BIT_PRINT8(xbits[i]); 
      /* fprintf(stdout,"%lld\n",ybits[i]); */
      continue;
    }
    else
    {
      xbits[i] = xbits[m] + xbits[i % m];
      ybits[i] = 2 * xbits[i];
      //BIT_PRINT8(xbits[i]);
      /*fprintf(stdout,"%lld\n",ybits[i]);*/
      continue;
    }
  }
  xbits1[0] = 0; xbits1[1] = 1;
  
  //fprintf(stdout,"%lld\n",xbits1[0]);
  //fprintf(stdout,"%lld\n",xbits1[1]);
  /*BIT_PRINT8(xbits[0]);
  BIT_PRINT8(xbits[1]);*/
  
  for(i = 2, m = 2, l = 2; i < xybits_size; i++)
  {
    k = i / m;
    
    if (k < 2)
    {
      xbits1[i] = xbits1[i - m];
    }
    else
    {
      if (k == 4)
      {
        xbits1[i] = xbits1[0];
        m *= 4;
        l *= 2;
      }
      else 
        xbits1[i] = xbits1[i - 2 * m] + l;
    }
   //fprintf(stdout,"%lld\n",xbits1[i]);
  }
  
  ybits1[0] = 0; ybits1[1] = 0;
  //fprintf(stdout,"%lld\n",ybits1[0]);
  //fprintf(stdout,"%lld\n",ybits1[1]);
  
  for(i = 2, m = 1, l = 1; i < xybits_size; i++)
  {
    k = i / m;
    
    if (k < 2)
    {
      ybits1[i] = ybits1[i - m];
    }
    else
    {
      if (k == 4)
      {
        ybits1[i] = ybits1[0];
        m *= 4;
        l *= 2;
      }
      else 
        ybits1[i] = ybits1[i - 2 * m] + l;
    }
   //fprintf(stdout,"%lld\n",ybits1[i]);
  }
}


/*  */
void q3c_dump_prm(struct q3c_prm *hprm,char *filename)
{
  FILE *fp = fopen(filename, "w");
  int i, x = 1 << q3c_interleaved_nbits;
  q3c_ipix_t *xbits = hprm->xbits, *ybits = hprm->ybits,
             *xbits1 = hprm->xbits1, *ybits1 = hprm->ybits1;
  fprintf(fp, "#include \"common.h\"\n/*struct q3c_prm \n{\n q3c_ipix_t nside;\nq3c_ipix_t *xbits;\nq3c_ipix_t *ybits;\nq3c_ipix_t *xbits1;\nq3c_ipix_t *ybits1;\n};*/");
  fprintf(fp, "\nq3c_ipix_t ____xbits[%d]={", x);
  fprintf(fp, " ");
  for(i = 0; i < x; i++)
  {
    if (i > 0) fprintf(fp, ",");
    fprintf(fp, Q3C_IPIX_FMT"U", xbits[i]);
  }
  fprintf(fp, "};");

  fprintf(fp, "\nq3c_ipix_t ____ybits[%d]={",x);
  fprintf(fp, " ");
  for(i = 0; i < x; i++)
  {
    if (i > 0) fprintf(fp, ",");
    fprintf(fp, Q3C_IPIX_FMT"U", ybits[i]);
  }
  fprintf(fp, "};");

  fprintf(fp, "\nq3c_ipix_t ____xbits1[%d]={", x);
  fprintf(fp, " ");
  for(i = 0; i < x; i++)
  {
    if (i > 0) fprintf(fp, ",");
    fprintf(fp, Q3C_IPIX_FMT"U", xbits1[i]);
  }
  fprintf(fp, "};");


  fprintf(fp, "\nq3c_ipix_t ____ybits1[%d]={",x);
  fprintf(fp, " ");
  for(i = 0; i < x; i++)
  {
    if (i > 0) fprintf(fp, ",");
    fprintf(fp, Q3C_IPIX_FMT"U", ybits1[i]);
  }
  fprintf(fp, "};\n");
  
  fprintf(fp, "struct q3c_prm hprm={%lld,____xbits,____ybits,____xbits1,____ybits1};\n", hprm->nside); 
  
  
}


/* Can be joined with ang2ipix_xy function */
/* That function compute the coefficients of the equation of the ellipse
 * (axx*x^2+ayy*y^2+2*axy*(x*y)+ax*x+ay*y+a=0)
 * produced on the cube face from the cone search
 */
void q3c_get_poly_coefs(char face_num, q3c_coord_t ra0, q3c_coord_t dec0, 
                        q3c_coord_t rad, q3c_coord_t *axx, q3c_coord_t *ayy,
                        q3c_coord_t *axy, q3c_coord_t *ax, q3c_coord_t *ay,
                        q3c_coord_t *a)
{
  q3c_coord_t ra1, dec1, sr, cr, cd, sd, crad, p = 1; 
  
  if ((face_num >= 1) && (face_num <= 4))
  {
    ra1 = (ra0 - (face_num  - 1 ) * 90) * q3c_DEGRA;
    dec1 = dec0 * q3c_DEGRA;    
    sr = q3c_sin(ra1);
    cr = q3c_cos(ra1);
    sd = q3c_sin(dec1);
    cd = q3c_cos(dec1);
    crad = q3c_cos(q3c_DEGRA * rad);
    *axx = (crad * crad - sr * sr * cd * cd);
    *ayy = (crad * crad - sd * sd);
    *axy = (-2 * sr * sd * cd);
    *ax = (-2 * sr * cr * cd * cd);
    *ay = (-2 * cr * sd * cd);
    *a = crad * crad - cr * cr * cd * cd;
  }
  else
  {
    if(face_num > 0) p = -1; 
    /* p=1 for North Polar cap & p=-1 for South Polar cap */
    
    ra1 = ra0 * q3c_DEGRA;
    dec1 = dec0 * q3c_DEGRA;
    sr = q3c_sin(ra1);
    cr = q3c_cos(ra1);
    sd = q3c_sin(dec1);
    cd = q3c_cos(dec1);
    crad = q3c_cos(q3c_DEGRA * rad);
    *axx = (crad * crad - sr * sr * cd * cd);
    *ayy = (crad * crad - cr * cr * cd * cd);
    *axy = (2 * p * sr * cr * cd * cd);
    *ax = -(2 * p * sr * sd * cd);
    *ay = (2 * cr * sd * cd);
    *a = crad * crad  - sd * sd;    
  }
  
  
  /* By this step I convert the coefficient of the polynome to the 
   * coordinate system on the cube face where x[-0.5,0.5] y[-0.5,0.5]
   * I should incorporate this formulae directly with the formulae 
   * from the top
   */
  *axx = (*axx) * 4; *ayy = (*ayy) * 4; 
  *axy = (*axy) * 4; 
  *ax = (*ax) * 2; *ay = (*ay) * 2; 
  
  //fprintf(stdout,"%Lfx^2 %Lfy^2 %Lfxy %Lfx %Lfy %Lf\n",*axx,*ayy,*axy,*ax,*ay,*a);
}


/* That function get the minimal, maximal x and y of the ellipse with the given
 * coefficients (axx,ayy,axy...). All the computations are done on the cube face.
 */
void q3c_get_xy_minmax(q3c_coord_t axx, q3c_coord_t ayy, q3c_coord_t axy, 
                       q3c_coord_t ax, q3c_coord_t ay, q3c_coord_t a,
                       q3c_coord_t *xmin, q3c_coord_t *xmax, q3c_coord_t *ymin,
                       q3c_coord_t *ymax)
{
  q3c_coord_t tmp0, tmp1, tmp2;
  
  tmp0 = axy * ax - 2 * axx * ay;
  tmp1 = 2 * q3c_sqrt(axx * (axx * ay * ay - axy * ax * ay - 
                            4 * axx * ayy * a + axy * axy * a + ax * ax * ayy));
  tmp2 = 4 * axx * ayy - axy * axy;
  *ymin = (tmp0 - tmp1) / tmp2;
  *ymax = (tmp0 + tmp1) / tmp2;
  tmp0 = axy * ay - 2 * ayy * ax;
  tmp1 = 2 * q3c_sqrt(ayy * (ayy * ax * ax - axy * ax * ay - 
                            4 * axx * ayy * a + axy * axy * a + ay * ay * axx));
  *xmin = (tmp0 - tmp1) / tmp2;
  *xmax = (tmp0 + tmp1) / tmp2;
  //fprintf(stdout,"xmin=%.10Lf xmax=%.10Lf ymin=%.10Lf ymax=%.10Lf\n", *xmin, *xmax, *ymin, *ymax);

}

/* That function get the minimal, maximal x and y of the ellipse with the given
 * coefficients (axx,ayy,axy...). All the computations are done on the cube face.
 * That function take as arguments only the ra, dec of the center of cone search 
 * and radius.
 */
void q3c_fast_get_circle_xy_minmax(char face_num, q3c_coord_t ra0, q3c_coord_t dec0, 
                            q3c_coord_t rad, q3c_coord_t *xmin, 
                            q3c_coord_t *xmax, q3c_coord_t *ymin,
                            q3c_coord_t *ymax)
{
  q3c_coord_t tmp0, tmp1, tmp2, ra1, dec1, sr, cr ,sd, cd, srad, crad, crad2, 
              cd2, scd;

  if ((face_num >= 1) && (face_num <= 4))
  {
    ra1 = (ra0 - (face_num  - 1 ) * 90) * q3c_DEGRA;
    dec1 = dec0 * q3c_DEGRA;    
    //cr = q3c_sqrt(1 - sr * sr);
#ifdef __USE_GNU
    q3c_sincos(ra1, &sr, &cr);
#else
    sr = q3c_sin(ra1);
    cr = q3c_cos(ra1);
#endif
    //sd = q3c_sin(dec1);
    //cd = q3c_sqrt( 1 - sd * sd);
    //cd = q3c_cos(dec1);
#ifdef __USE_GNU
    q3c_sincos(dec1, &sd, &cd);
#else
    sd = q3c_sin(dec1);
    cd = q3c_cos(dec1);
#endif
    cd2 = cd * cd;
    //srad = q3c_sin(q3c_DEGRA * rad);
    //crad = q3c_sqrt(1 - srad * srad);
    //crad = q3c_cos(q3c_DEGRA * rad);
#ifdef __USE_GNU
    q3c_sincos(q3c_DEGRA * rad, &srad, &crad);
#else
    srad = q3c_sin(q3c_DEGRA * rad);
    crad = q3c_cos(q3c_DEGRA * rad);
#endif

    tmp2 = ((q3c_coord_t)1) / (2 * ( cd2 * cr * cr - srad * srad ));
    tmp0 = sr * cr *cd2;
    tmp1 = srad * q3c_sqrt(cd2 - srad * srad);
    *xmin = (tmp0 - tmp1) * tmp2;
    *xmax = (tmp0 + tmp1) * tmp2;
    tmp0 = cr * cd * sd;
    tmp1 = srad * q3c_sqrt(crad * crad - cd2 * sr * sr);
    *ymin = (tmp0 - tmp1) * tmp2;
    *ymax = (tmp0 + tmp1) * tmp2;
  }
  else 
  {
    ra1 = ra0 * q3c_DEGRA;
    dec1 = dec0 * q3c_DEGRA;
//    sr = q3c_sin(ra1);
//    cr = q3c_cos(ra1);
#ifdef __USE_GNU
    q3c_sincos(ra1, &sr, &cr);
#else
    sr = q3c_sin(ra1);
    cr = q3c_cos(ra1);
#endif

#ifdef __USE_GNU
    q3c_sincos(dec1, &sd, &cd);
#else
    sd = q3c_sin(ra1);
    cd = q3c_cos(ra1);
#endif

    cd2 = cd * cd;
    scd = sd * cd;
    //sd = q3c_sin(dec1);
//    srad = q3c_sin(q3c_DEGRA * rad);
//    crad = q3c_sqrt(1 - srad * srad);

#ifdef __USE_GNU
    q3c_sincos(q3c_DEGRA * rad, &srad, &crad);
#else
    srad = q3c_sin(q3c_DEGRA * rad);
    crad = q3c_cos(q3c_DEGRA * rad);
#endif

    crad2 = crad * crad;
    tmp0 = scd * sr;
    tmp1 = srad * q3c_sqrt(crad2 - cr * cr * cd2);
    tmp2 = ((q3c_coord_t)1) / (2 * (crad2 - cd2));
    if (face_num == 5) tmp0 = -tmp0;

    *xmin = (tmp0 - tmp1) * tmp2;
    *xmax = (tmp0 + tmp1) * tmp2;
    tmp0 = - scd * cr;
    tmp1 = srad * q3c_sqrt(crad2 - sr * sr * cd2);
    *ymin = (tmp0 - tmp1) * tmp2;
    *ymax = (tmp0 + tmp1) * tmp2;
  }
}

void q3c_get_equatorial_ellipse_xy_minmax(q3c_coord_t alpha, q3c_coord_t delta,
	q3c_coord_t d, q3c_coord_t e, q3c_coord_t PA, q3c_coord_t *ymin,
	q3c_coord_t *ymax, q3c_coord_t *zmin, q3c_coord_t *zmax)
{
/* Thank you, Maple! */
	q3c_coord_t      t1 = q3c_sin(alpha);
	q3c_coord_t      t2 = q3c_cos(alpha);
	q3c_coord_t      t21 = q3c_sin(delta);
	q3c_coord_t      t4 = q3c_cos(delta);	
	q3c_coord_t      t24 = q3c_sin(PA);
	q3c_coord_t      t13 = q3c_cos(PA);
	q3c_coord_t      t51 = q3c_sin(d);	
	q3c_coord_t      t8 = q3c_cos(d);

	q3c_coord_t      t3 = t1*t2;
	q3c_coord_t      t5 = t4*t4;
	q3c_coord_t      t7 = 2.0*t3*t5;
	q3c_coord_t      t9 = t8*t8;
	q3c_coord_t      t12 = t1*t5;
	q3c_coord_t      t14 = t13*t13;
	q3c_coord_t      t15 = t2*t14;
	q3c_coord_t      t22 = t21*t13;
	q3c_coord_t      t23 = t2*t2;
	q3c_coord_t      t34 = t5*t9;
	q3c_coord_t      t39 = t24*t9;
	q3c_coord_t      t47 = -t7-2.0*t3*t9+2.0*t12*t15-2.0*t12*t15*t9+4.0*t22*t23*t24-4.0*t15*t1+4.0*t1*t9*t15+2.0*t3+2.0*t3*t34-2.0*t22*t24+2.0*t22*t39-4.0*t21*t9*t13*t23*t24;
	q3c_coord_t      t48 = e*e;
	q3c_coord_t      t52 = t51*t51;
	q3c_coord_t      t54 = t5*t14*t9;
	q3c_coord_t      t61 = t23*t5;
	q3c_coord_t      t62 = 2.0*t61;
	q3c_coord_t      t63 = t23*t14;
	q3c_coord_t      t67 = t23*t9;
	q3c_coord_t      t69 = t61*t9;
	q3c_coord_t      t71 = t14*t9;
	q3c_coord_t      t73 = t1*t21;
	q3c_coord_t      t77 = t73*t13*t2*t24*t9;
	q3c_coord_t      t79 = t71*t23;
	q3c_coord_t      t85 = t63*t34;
	q3c_coord_t      t89 = -t62-4.0*t63+2.0*t63*t5-2.0*t67+2.0*t69-2.0*t71+4.0*t77+4.0*t79-4.0*t73*t13*t2*t24-2.0*t85+2.0*t14+2.0*t23;
	q3c_coord_t      t92 = t89*t48-2.0+2.0*t9+t62;
	q3c_coord_t      t93 = t1*t13;
	q3c_coord_t      t96 = t21*t2;
	
	q3c_coord_t      tmpy0 = t47*t48+t7;
	q3c_coord_t      tmpy1 = -4.0*t52*(t9-1.0+t54+t5-t34)*t48+4.0*t52*(-1.0+t9+t5);
	q3c_coord_t      tmpy2 = t92;

	tmpy1 = q3c_sqrt(tmpy1);
	tmpy2 = (2 * tmpy2);

	q3c_coord_t      tmpz0 = -2.0*(-t93*t24+t93*t39+t96+t96*t71-t96*t14-t96*t9)*t4*t48+2.0*t96*t4;
	q3c_coord_t      tmpz1 = -4.0*t52*(t61-2.0*t79-t54+t67-2.0*t77-t5-t69+t71+t85+t34)*t48+4.0*t52*(t9+t61-t5);
	q3c_coord_t      tmpz2 = t92;

	tmpz1 = q3c_sqrt(tmpz1);
	tmpz2 = (2 * tmpz2);

	*ymin = (tmpy0 - tmpy1) / tmpy2;
	*ymax = (tmpy0 + tmpy1) / tmpy2;
	*zmin = (tmpz0 - tmpz1) / tmpz2;
	*zmax = (tmpz0 + tmpz1) / tmpz2;

}

/*north and south pole 
for south pole the ycoordinates (1st) should be inverted*/

void q3c_get_polar_ellipse_xy_minmax(q3c_coord_t alpha, q3c_coord_t delta,
	q3c_coord_t d, q3c_coord_t e, q3c_coord_t PA, q3c_coord_t *ymin,
	q3c_coord_t *ymax, q3c_coord_t *zmin, q3c_coord_t *zmax)
{
/* Thank you, Maple! */
	q3c_coord_t      t1 = q3c_sin(alpha);
	q3c_coord_t      t14 = q3c_cos(alpha);	
	q3c_coord_t      t2 = q3c_sin(delta);
	q3c_coord_t      t19 = q3c_cos(delta);	
	q3c_coord_t      t12 = q3c_sin(PA);	
	q3c_coord_t      t7 = q3c_cos(PA);
	q3c_coord_t      t25 = q3c_sin(d);
	q3c_coord_t      t4 = q3c_cos(d);

	q3c_coord_t      t3 = t1*t2;	
	q3c_coord_t      t5 = t4*t4;
	q3c_coord_t      t6 = t3*t5;
	q3c_coord_t      t8 = t7*t7;
	q3c_coord_t      t10 = t8*t5;
	q3c_coord_t      t13 = t7*t12;
	q3c_coord_t      t15 = t13*t14;
	q3c_coord_t      t21 = e*e;
	q3c_coord_t      t26 = t25*t25;
	q3c_coord_t      t28 = 2.0*t6*t15;
	q3c_coord_t      t29 = t19*t19;
	q3c_coord_t      t30 = t14*t14;
	q3c_coord_t      t31 = t29*t30;
	q3c_coord_t      t32 = t31*t5;
	q3c_coord_t      t34 = 2.0*t10*t30;
	q3c_coord_t      t35 = t30*t5;
	q3c_coord_t      t36 = t31*t10;
	q3c_coord_t      t46 = t29*(1.0-t5-t8+t10)*t21+t5-t29;
	q3c_coord_t      t47 = t7*t1;
	q3c_coord_t      t51 = t14*t2;

	q3c_coord_t      tmpy0 = 2.0*(t6+t3*t8-t3*t10-t15+t13*t14*t5-t3)*t19*t21+2.0*t3*t19;
	q3c_coord_t      tmpy1 = 4.0*t26*(-t5-t28-t32-t34+t35+t36+t31+t10)*t21-4.0*t26*(-t5+t31);
	q3c_coord_t      tmpy2 = 2.0*t46;

	tmpy1 = q3c_sqrt(tmpy1);
	tmpy2 = (2 * tmpy2);

	q3c_coord_t      tmpz0 = 2.0*(-t47*t12+t47*t12*t5+t51*t10-t51*t5-t51*t8+t51)*t19*t21-2.0*t51*t19;
	q3c_coord_t      tmpz1 = -4.0*t26*(-t28-t29*t8*t5-t29-t32-t34+t35+t36+t31+t29*t5+t10)*t21+4.0*t26*(t5-t29+t31);
	q3c_coord_t      tmpz2 = 2.0*t46;

	tmpz1 = q3c_sqrt(tmpz1);
	tmpz2 = (2 * tmpz2);

	*ymin = (tmpy0 - tmpy1) / tmpy2;
	*ymax = (tmpy0 + tmpy1) / tmpy2;
	*zmin = (tmpz0 - tmpz1) / tmpz2;
	*zmax = (tmpz0 + tmpz1) / tmpz2;
}


void q3c_fast_get_ellipse_xy_minmax(char face_num, q3c_coord_t ra0, q3c_coord_t dec0, 
                            q3c_coord_t rad0, q3c_coord_t e, q3c_coord_t PA0,
                            q3c_coord_t *xmin, q3c_coord_t *xmax,
                            q3c_coord_t *ymin, q3c_coord_t *ymax)
{
	q3c_coord_t ra1 = ra0 * q3c_DEGRA, dec1 = dec0 * q3c_DEGRA,
			rad1 = rad0 * q3c_DEGRA, PA1 = PA0 * q3c_DEGRA,
			tmpx;
	if ((face_num > 0) && (face_num < 5))
	{
		q3c_get_equatorial_ellipse_xy_minmax(ra1, dec1, rad1, e, PA1,
						xmin, xmax, ymin, ymax);
	}
	else
	{
		q3c_get_polar_ellipse_xy_minmax(ra1, dec1, rad1, e, PA1,
						xmin, xmax, ymin, ymax);
		if (face_num==5)
		{
			tmpx = *xmin;
			*xmin = - (*xmax);
			*xmax = tmpx;
		}
	}
}



/* Function cheching whether the square with center xc_cur, yc_cur and the 
 * size cur_size on the cube face lie inside, or intersects etc. with the ellipse
 * specified by the coefficients (axx, axy, ayy, ax, ay, a)
 */
inline char q3c_circle_cover_check(q3c_coord_t xc_cur, q3c_coord_t yc_cur,
                                   q3c_coord_t cur_size, q3c_coord_t xmin,
                                   q3c_coord_t xmax, q3c_coord_t ymin, 
                                   q3c_coord_t ymax, q3c_coord_t axx,
                                   q3c_coord_t axy, q3c_coord_t ayy,
                                   q3c_coord_t ax, q3c_coord_t ay,
                                   q3c_coord_t a)
{
    q3c_coord_t xl_cur, xr_cur, yb_cur, yt_cur, x_cur, y_cur, val;

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



    /* UNDEF_CHECK00: */
    /* Bottom left vertex */
    x_cur = xl_cur;
    y_cur = yb_cur;
    val = x_cur * (axx * x_cur + axy * y_cur + ax) +
          y_cur * (ayy * y_cur + ay) + a;
    
    if (val < 0) goto PARTUNDEF_CHECK01;
    

    /* UNDEF_CHECK01: */
    /* Bottom right vertex */
    x_cur = xr_cur;
    val = x_cur * (axx * x_cur + axy * y_cur + ax) +
          y_cur * (ayy * y_cur + ay) + a;
    
    if (val < 0) 
    {
      return q3c_PARTIAL;
    }
    
    /* UNDEF_CHECK10: */
    /* Top right vertex */
    y_cur = yt_cur;
    val = x_cur * (axx * x_cur + axy * y_cur + ax) +
          y_cur * (ayy * y_cur + ay) + a;
          
    if (val < 0) 
    {
      return q3c_PARTIAL;
    }


    /* UNDEF_CHECK11: */
    /* Top left vertex */
    x_cur = xl_cur;
    val = x_cur * (axx * x_cur + axy * y_cur + ax) +
          y_cur * (ayy * y_cur + ay) + a;
          
    if (val < 0) 
    {
      return q3c_PARTIAL;
    }
    else 
    {
      /* Testing if the ellipse crosses the borders of the box)
       * OR the box covers the whole ellipse ( this is expressed by the last
       * condition (center of the ellipse is inside the box))
       */
      
      
      if (
            (q3c_INTERSECT(xmin, xmax, xl_cur, xr_cur) &&
             q3c_INTERSECT(ymin, ymax, yb_cur, yt_cur)
            )&&
          ((((2 * axx * xl_cur + axy * yt_cur + ax) * 
            (2 * axx * xr_cur + axy * yt_cur + ax)) < 0) ||
          (((2 * axx * xl_cur + axy * yb_cur + ax) *
            (2 * axx * xr_cur + axy * yb_cur + ax)) < 0) ||
          (((2 * ayy * yb_cur + axy * xl_cur + ay) *
            (2 * ayy * yt_cur + axy * xl_cur + ay)) < 0) ||
          (((2 * ayy * yb_cur + axy * xr_cur + ay) *
            (2 * ayy * yt_cur + axy * xr_cur + ay)) < 0)
          ))
      {
        
        return q3c_PARTIAL;
      }
      else 
      {
        return q3c_DISJUNCT;
      }
    }
  //((x0<xr_cur)&&(x0>xl_cur)&&(y0<yt_cur)&&(y0>yb_cur))

    /* PARTUNDEF labels -- when we know the ellipse have 
     *   at least a partial intersection with the box  
     */
    

    PARTUNDEF_CHECK01:
    /* Bottom right vertex */
    x_cur = xr_cur;
    val = x_cur * (axx * x_cur + axy * y_cur + ax) +
          y_cur * (ayy * y_cur + ay) + a;
    
    if (val >= 0) 
    {
      return q3c_PARTIAL;
    }


    /* PARTUNDEF_CHECK10: */
    /* Top right vertex */
    y_cur = yt_cur;
    val = x_cur * (axx * x_cur + axy * y_cur + ax) +
          y_cur * (ayy * y_cur + ay) + a;
    if (val >= 0) 
    {
      return q3c_PARTIAL;
    }


    /* PARTUNDEF_CHECK11: */
    /* Top left vertex */
    x_cur = xl_cur;
    val = x_cur * (axx * x_cur + axy * y_cur + ax) +
          y_cur * (ayy * y_cur + ay) + a;
    if (val >= 0) 
    {
      return q3c_PARTIAL;
    }
    else
    {
      return q3c_COVER;
    }

}



void q3c_radial_query(struct q3c_prm *hprm, char *table_name,
                      char *ra_col_name, char *dec_col_name, q3c_coord_t ra0,
                      q3c_coord_t dec0, q3c_coord_t rad, char * qstring)
{
  
  
  q3c_coord_t axx, ayy, axy, ax, ay, a, xmin, xmax, ymin, ymax, 
              xc_cur = 0 , yc_cur = 0, cur_size, xesize, yesize, xtmp, ytmp, 
              points[4];
                    
  q3c_ipix_t n0, nside=hprm->nside, ixmin, iymin, ixmax, iymax, ntmp,
              ntmp1, xi, yi, ipix_tmp1, ipix_tmp2, *xbits=hprm->xbits, 
              *ybits=hprm->ybits, i1;
  
  char face_num, multi_flag = 0, k, where_part[30000], 
      where_cover[30000], radius_formulae[1000],
      ipix_col_name[256]="ipix", face_count, face_num0,
      q3c_sindist_func_name[]="q3c_sindist";


/* Old, low precision and/or SQL formulae   
  sprintf(radius_formulae, "cos(RADIANS(dec))*cos(RADIANS("Q3C_COORD_FMT"))*(cos(RADIANS(ra-"Q3C_COORD_FMT")))+sin(RADIANS(dec))*sin(RADIANS("Q3C_COORD_FMT"))", 
           dec0, ra0, dec0);
   sprintf(radius_formulae, "2 * ASIN(SQRT(POW(SIN(RADIANS((%s-("Q3C_COORD_FMT"))/2)),2)+POW(SIN(RADIANS((%s-("Q3C_COORD_FMT"))/2)),2)*COS(RADIANS(%s))*COS(RADIANS("Q3C_COORD_FMT"))))",dec_col_name,dec0,ra_col_name,ra0,dec_col_name,dec0);
   sprintf(radius_formulae, "(POW(SIN(RADIANS((%s-("Q3C_COORD_FMT"))/2)),2)+POW(SIN(RADIANS((%s-("Q3C_COORD_FMT"))/2)),2)*COS(RADIANS(%s))*COS(RADIANS("Q3C_COORD_FMT")))",dec_col_name,dec0,ra_col_name,ra0,dec_col_name,dec0);
  */

  sprintf(radius_formulae, "%s(%s,%s,"Q3C_COORD_FMT","Q3C_COORD_FMT")", q3c_sindist_func_name, ra_col_name, dec_col_name, ra0, dec0);
  
  int work_nstack = 0, i, j, tmp_stack1, tmp_stack2, out_nstack = 0,
      cover_len = 0, part_len = 0, res_depth;
  
  const int max_depth = 6; /* log2(Maximal increase of resolution) */
  
  struct q3c_square work_stack[11024], out_stack[11024], *cur_square;
  /* !!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!! 
   * Consider that the size of the stacks should directly depend on the
   * value of res_depth variable !
   * It seems that each of stacks should have the size 3*4*(2^(depth+1)-1)
   */
  
  where_part[0] = 0;
  where_cover[0] = 0;
  
  face_num = q3c_getfacenum(ra0, dec0);
  
  q3c_get_poly_coefs(face_num, ra0, dec0, rad, &axx, &ayy, &axy, &ax, &ay, &a);
  /* The coefficients of the polynome are obtained for the projection 
   * on the cube face for the cube with the edge length 1
   * axx*x^2+ayy*y^2+axy*x*y+ax*x+ay*y+a
   */ 

  q3c_get_xy_minmax(axx, ayy, axy, ax, ay, a, &xmin, &xmax, &ymin, &ymax);
  

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
  
  for(face_count = 0; face_count <= multi_flag; out_nstack = 0, face_count++)
  {
    /* This the beginning of the mega-loop over multiple faces */
    
    if (face_count > 0)
      /* This "if" works when we pass through the secondary faces */ 
    {
      face_num = q3c_xy2facenum(2 * points[2 * (face_count - 1)],
                    2 * points[2 * (face_count - 1) + 1], face_num0);
      q3c_get_poly_coefs(face_num, ra0, dec0, rad, &axx, &ayy, &axy, &ax, &ay, &a);
      
      q3c_get_xy_minmax(axx, ayy, axy, ax, ay, a, &xmin, &xmax, &ymin, &ymax);
      
      xmax = (xmax > q3c_HALF ? q3c_HALF : xmax);
      xmin = (xmin < -q3c_HALF ? -q3c_HALF : xmin);
      ymax = (ymax > q3c_HALF ? q3c_HALF : ymax);
      ymin = (ymin < -q3c_HALF ? -q3c_HALF : ymin);
    }
    
#ifdef Q3C_DEBUG
    fprintf(stdout,"FACE RUN: %d FACE_NUM: %d\n", face_count, face_num);
#endif
    
    xesize = xmax - xmin; 
    yesize = ymax - ymin;
    xesize = xesize > yesize ? xesize : yesize;

    if (xesize * nside < 1) 
    /* If the region is too small */
    {
      xesize=1 / (q3c_coord_t)nside;
    }
    
    n0 = 1 << ((q3c_ipix_t)(-q3c_ceil((q3c_log(xesize) / q3c_log(2)))));  
    /* n0 is now the level of quadtree for which the minimal 
     * element is >~ our ellipse
     */

    cur_size = ((q3c_coord_t)1) / n0;
    //fprintf(stdout, "%lld %Lf %Lf %Lf\n", n0,cur_size,xesize,yesize);
#ifdef Q3C_DEBUG
    fprintf(stdout,"XMIN: "Q3C_COORD_FMT" XMAX: "Q3C_COORD_FMT" YMIN: "Q3C_COORD_FMT" YMAX: "Q3C_COORD_FMT"\n", xmin, xmax, ymin, ymax);
#endif

    /* Here we set up the stack with initial squares */
    
    ixmin = (q3c_HALF + xmin) * n0; /* Here I use the C truncation of floats */
    ixmax = (q3c_HALF + xmax) * n0; /* to integers */
    iymin = (q3c_HALF + ymin) * n0;
    iymax = (q3c_HALF + ymax) * n0;
    
    
    ixmax = (ixmax == n0 ? n0-1 : ixmax);
    iymax = (iymax == n0 ? n0-1 : iymax);
    
    
    cur_square = work_stack;
    if (ixmin == ixmax) 
    {
      if (iymin == iymax)
      {
        cur_square->x0 = ixmin;
        cur_square->y0 = iymin;
        cur_square->nside0 = n0;
        work_nstack = 1;
      }
      else
      {
        cur_square->x0 = ixmin;
        cur_square->y0 = iymin;
        cur_square->nside0 = n0;
        cur_square++;
        cur_square->x0 = ixmin;
        cur_square->y0 = iymax;
        cur_square->nside0 = n0; 
        work_nstack = 2;
      }
    }
    else
    {
      if (iymin == iymax)
      {
        cur_square->x0 = ixmin;
        cur_square->y0 = iymin;
        cur_square->nside0 = n0;
        cur_square++;
        cur_square->x0 = ixmax;
        cur_square->y0 = iymin;
        cur_square->nside0 = n0;      
        work_nstack = 2;
      }
      else
      {
        cur_square->x0 = ixmin;
        cur_square->y0 = iymin;
        cur_square->nside0 = n0;
        cur_square++;
        cur_square->x0 = ixmin;
        cur_square->y0 = iymax;
        cur_square->nside0 = n0; 
        cur_square++;
        cur_square->x0 = ixmax;
        cur_square->y0 = iymin;
        cur_square->nside0 = n0;
        cur_square++;
        cur_square->x0 = ixmax;
        cur_square->y0 = iymax;
        cur_square->nside0 = n0;
        work_nstack = 4;
      }
    }
    
    
    /* For this case the maximal increase of resolution of 2^res_depth
     * for each axis 
     */
    
    res_depth = nside / n0;
    /* If the the query is too small we cannot go up to max_depth since we
     * are limited by nside depth
     */
    res_depth = max_depth > res_depth ? res_depth : max_depth;

    for(i = 1; i <= res_depth; i++)
    {
#ifdef Q3C_DEBUG
      fprintf(stdout,"SQUARE RUN %d :\n", i);
#endif
      
      /* This loop perform the testing of all squares in work_stack */
      for(j = 0; j < work_nstack; j++)
      {
        cur_square = work_stack + j;
        cur_size=((q3c_coord_t) 1) / (cur_square->nside0);
        xc_cur = (( (q3c_coord_t) cur_square->x0) + q3c_HALF) / cur_square->nside0 - q3c_HALF;
        yc_cur = (( (q3c_coord_t) cur_square->y0) + q3c_HALF) / cur_square->nside0 - q3c_HALF;
        /* xc_cur and yc_cur -- center of the square (in the coordinate system
         * of big square [-0.5:0.5]x[-0.5:0.5]
         */
//        goto CHECK_BOX;
//        END_CHECK_BOX:
        //fprintf(stdout,"%Lf %Lf %Lf %d\n",xc_cur,yc_cur,cur_size, status);
        cur_square->status = q3c_circle_cover_check(xc_cur, yc_cur, cur_size,
                                                    xmin, xmax, ymin, ymax, 
                                                    axx, axy, ayy, ax, ay, a);
      }
      
#ifdef Q3C_DEBUG
      fprintf(stdout,"NUM squares in the stack %d\n",work_nstack);
#endif
      /* Now we select the fully covered set of squares from stack and put them
       * into out_stack, the partly covered squares are expanded to corresponding
       * set of 4 squares each.
       * explanation of the following scheme
       *    |xxxxxxxxxxxxx     xxxxxxxxxx|
       *                                 ^
       *                  /---/           tmp_stack2
       *                tmp_stack1
       */ 
      for(j = 0, tmp_stack1 = 0, tmp_stack2 = work_nstack; j < work_nstack; j++)
      {
        cur_square = work_stack + j;
        //fprintf(stdout,"%d %d %d\n",work_nstack,tmp_stack1,tmp_stack2);
        if (cur_square->status == q3c_PARTIAL)
        /* If this square partially intersects with the ellipse
         * I should split this square farther
         */
        {

          /* If this is the last stage of resolution loop, I will not split
           * the "partial" boxes
           */
          if (i == res_depth) continue; 
          tmp_stack1++;

          xtmp = 2 * cur_square->x0;
          ytmp = 2 * cur_square->y0;
          ntmp = 2 * cur_square->nside0;
          
          /* First I try to put the childrens of this square in the part of
           * the stack freed by trown away squares (which were disjunct from
           * the ellipse or which were fully covered by the ellipse)
           */ 
          for(k = 0; (k <= 3) && (tmp_stack1 > 0); k++)
          {
            cur_square = work_stack + (j + 1 - tmp_stack1);
            cur_square->x0 = xtmp + (k & 1);
            cur_square->y0 = ytmp + ((k & 2) >> 1);
            cur_square->nside0 = ntmp;
            tmp_stack1--;
          }
          
          for (; k <= 3; k++)
          {
            cur_square = work_stack + tmp_stack2;
            cur_square->x0 = xtmp + (k & 1);
            cur_square->y0 = ytmp + ((k & 2) >> 1);
            cur_square->nside0 = ntmp;
            tmp_stack2++;
          }
          
        }
        else 
        {
          if (cur_square->status == q3c_COVER)
          /* I put this square in the output list and 
           * free one place in the stack
           */
          {
            out_stack[out_nstack++] = *cur_square;
            tmp_stack1++;
          }
          else 
          /* This branch can be reached only if status==q3c_DISJUNCT */
          {
            tmp_stack1++;
            /* I just drop this square and free the place in the stack */
          }
        } 
      
      } /* end of updating of the list of squares loop */ 
      
#ifdef Q3C_DEBUG
      fprintf(stdout,"STACK STATE nw_stack: %d nt_stack1: %d nt_stack2: %d\n", work_nstack, tmp_stack1, tmp_stack2);
#endif
      
      
      if (i == res_depth) break; 
      /* After updating the list of squares I compute how much I have them now 
       * (except for the case of last resolution step) 
       */
      if (tmp_stack1 == 0) 
      {
        work_nstack = tmp_stack2;
      } 
      else 
      {
        if ((tmp_stack2-work_nstack) > tmp_stack1)
          {
          memcpy(work_stack + (work_nstack - tmp_stack1), work_stack + (tmp_stack2 - tmp_stack1), tmp_stack1 * sizeof(struct q3c_square)); 
          work_nstack = tmp_stack2 - tmp_stack1;
          }
        else
          {
          memcpy(work_stack + (work_nstack - tmp_stack1), work_stack + (tmp_stack2 - tmp_stack1), (tmp_stack2 - work_nstack) * sizeof(struct q3c_square));
          work_nstack = tmp_stack2 - tmp_stack1;
          }
      }
      
    } /* end of resolution loop */
    
    
    //   Old printing of the results
    
    for(i = 0; i < out_nstack; i++)
    {
      cur_square = out_stack + i;
#ifdef Q3C_DEBUG
      fprintf(stdout, "OUT: %f %f %d %d\n", cur_square->x0+0.5,cur_square->y0+0.5,cur_square->nside0,cur_square->status);
#endif
    }
    //fprintf(stdout,"XXX%d %d %d\n",work_nstack, tmp_stack1, tmp_stack2);
    
    for(i = 0; i < work_nstack; i++)
    {
      cur_square = work_stack + i;
#ifdef Q3C_DEBUG
      if (cur_square->status == q3c_PARTIAL)
        fprintf(stdout, "OUT1: %f %f %d %d\n", cur_square->x0+0.5,cur_square->y0+0.5,cur_square->nside0,cur_square->status);
#endif
    }
    
    
    
    ntmp = ((q3c_ipix_t) face_num) * nside * nside; 
    i1 = 1 << q3c_interleaved_nbits;
    
    /* Run through fully covered squares (we take them from out_stack) */ 
    for(i = 0; i < out_nstack; i++)
    {
      cur_square = out_stack + i;  
      ntmp1 = (nside / cur_square->nside0);  
      //fprintf(stdout, "XX%lld\n", ntmp1);
      xi = cur_square->x0 * ntmp1;
      yi = cur_square->y0 * ntmp1;
      

      /* Here we compute the ipix value for the bottom lower corner of the square */
  #ifdef Q3C_INT4 
      {
        ipix_tmp1 = ntmp + xbits[xi % i1] + ybits[yi % i1];
        /*4byte computation*/
      }
  #endif /* Q3C_INT4 */
  #ifdef Q3C_INT8
      {
        ipix_tmp1 = ntmp + xbits[xi % i1] + ybits[yi % i1] +
        (xbits[(xi >> q3c_interleaved_nbits) % i1] + ybits[(yi >> q3c_interleaved_nbits) % i1]) * i1 * i1;
        /*8byte computation*/
      }
  #endif /* Q3C_INT8 */
      
      ipix_tmp2=ipix_tmp1+(ntmp1*ntmp1);


      
      /* Now we have in ipix_tmp1 and ipix_tmp2 -- the pixel range for the 
       * query of current square 
       * The query should be     ipix_tmp1 =< II < ipix_tmp2  
       */
      
      if ((i == 0) && (face_count == 0))
      {
      /* This clause create the beginning of the where_cover query */
	cover_len += sprintf(where_cover + cover_len, "(%s>="Q3C_IPIX_FMT" AND %s<"Q3C_IPIX_FMT")\n", 
                      ipix_col_name, ipix_tmp1, ipix_col_name, ipix_tmp2); 
      }
      else 
      {
        cover_len += sprintf(where_cover + cover_len, "OR (%s>="Q3C_IPIX_FMT" AND %s<"Q3C_IPIX_FMT")\n", 
                      ipix_col_name, ipix_tmp1, ipix_col_name, ipix_tmp2);
      }
      //fprintf(stdout,"%d\n",len);
    
    } /* End of output run through fully covered squares */
    
    if (out_nstack == 0) 
    /* If the list of fully covered squares is empty */
    {
      if (face_count == 0)
      {	
        cover_len += sprintf(where_cover + cover_len, "false\n");
      }
      else
      {
        cover_len += sprintf(where_cover + cover_len, "OR false\n");      
      }
    }


    /* Run through partly covered squares (we take them from work_stack where
     * the cur_square->status == q3c_PARTIAL) 
     */ 
    for(i = 0, j = -1; i < work_nstack; i++)
    {
      cur_square = work_stack + i;
      if (cur_square->status!=q3c_PARTIAL) 
        continue;
      else 
        j+=1;  
      ntmp1 = (nside / cur_square->nside0);  
      //fprintf(stdout, "XX%lld\n", ntmp1);
      xi = cur_square->x0 * ntmp1;
      yi = cur_square->y0 * ntmp1;
      

      /* Here we compute the ipix value for the bottom lower corner of the square */
  #ifdef Q3C_INT4 
      {
        ipix_tmp1 = ntmp + xbits[xi % i1] + ybits[yi % i1];
        /*4byte computation*/
      }
  #endif /* Q3C_INT4 */
  #ifdef Q3C_INT8
      {
        ipix_tmp1 = ntmp + xbits[xi % i1] + ybits[yi % i1] +
        (xbits[(xi >> q3c_interleaved_nbits) % i1] + ybits[(yi >> q3c_interleaved_nbits) % i1]) * i1 * i1;
        /*8byte computation*/
      }
  #endif /* Q3C_INT8 */
      
      ipix_tmp2=ipix_tmp1+(ntmp1*ntmp1);


      
      /* Now we have in ipix_tmp1 and ipix_tmp2 -- the pixel range for the 
       * query of current square 
       * The query should be     ipix_tmp1 =< II < ipix_tmp2  
       */

      if ((j==0)&&(face_count==0))
      {
        part_len += sprintf(where_part + part_len, "((%s>="Q3C_IPIX_FMT" AND %s<"Q3C_IPIX_FMT")\n", 
                        ipix_col_name, ipix_tmp1, ipix_col_name, ipix_tmp2); 
      }
      else 
      {
        part_len += sprintf(where_part + part_len, "OR (%s>="Q3C_IPIX_FMT" AND %s<"Q3C_IPIX_FMT")\n", 
                       ipix_col_name, ipix_tmp1, ipix_col_name, ipix_tmp2);
      }
      
    
    } /* End of output run through partly covered squares */

  } /* End of the mega-loop over the faces */

  part_len += sprintf(where_part + part_len, ")");  
  
  
#ifdef Q3C_DEBUG
  fprintf(stderr, "COVER:%s\n", where_cover);
  fprintf(stderr, "PARTLY:%s\n", where_part);
#endif

  
  sprintf(qstring, "SELECT * FROM %s WHERE %s UNION ALL SELECT * FROM %s WHERE %s AND %s<POW(SIN(RADIANS("Q3C_COORD_FMT")/2),2)", 
          table_name, where_cover, table_name, where_part, radius_formulae, rad);
  
/*  Old (with old formulae)
    sprintf(qstring, "SELECT * FROM %s WHERE %s UNION ALL SELECT * FROM %s WHERE %s) AND %s>cos(RADIANS("Q3C_COORD_FMT"))", 
            table_name, where_cover, table_name, where_part, radius_formulae, rad);
*/


//  sprintf(qstring, "SELECT * FROM %s WHERE %s UNION ALL SELECT * FROM %s WHERE %s) AND %s>cos(RADIANS(%Lf))", 
//          table_name, where_cover, table_name, where_part, radius_formulae, rad);
 //fprintf(stderr,"%s",qstring);
//  fprintf(stdout,"%s",radius_formulae);
  //union all select * from healpix where ");

} /* End of radial_query() */








void q3c_new_radial_query(struct q3c_prm *hprm, q3c_coord_t ra0,
                          q3c_coord_t dec0, q3c_coord_t rad,
                          q3c_ipix_t *out_ipix_arr_fulls,
                          q3c_ipix_t *out_ipix_arr_partials
                          )
{
  
  
  q3c_coord_t axx, ayy, axy, ax, ay, a, xmin, xmax, ymin, ymax, 
              xc_cur = 0 , yc_cur = 0, cur_size, xesize, yesize, xtmp, ytmp, 
              points[4];
                    
  q3c_ipix_t n0, nside = hprm->nside, ixmin, iymin, ixmax, iymax, ntmp,
              ntmp1, xi, yi, ipix_tmp1, ipix_tmp2, *xbits = hprm->xbits, 
              *ybits = hprm->ybits, i1;
  
  char face_num, multi_flag = 0, k, face_count, face_num0;
  int out_ipix_arr_fulls_pos = 0;
  int out_ipix_arr_partials_pos = 0;
  int out_ipix_arr_fulls_length = 100;//1600;
  int out_ipix_arr_partials_length = 100;//1600;
  /* !!!!!!!!IMPORTANT!!!! 
   * Keep in mind that those lengths are in fact multiplied by two 
   * maximal number of squares 
   */

  
  int work_nstack = 0, i, j, tmp_stack1, tmp_stack2, out_nstack = 0,
      res_depth;
  
  const int max_depth = 4; /* log2(Maximal increase of resolution) */
  
  struct q3c_square work_stack[11024], out_stack[11024], *cur_square;
  /* !!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!! 
   * Consider that the size of the stacks should directly depend on the
   * value of res_depth variable !
   * It seems that each of stacks should have the size 4*(2^(depth-1))
   */
  
  face_num = q3c_getfacenum(ra0, dec0);
  
  q3c_get_poly_coefs(face_num, ra0, dec0, rad, &axx, &ayy, &axy, &ax, &ay, &a);
  /* The coefficients of the polynome are obtained for the projection 
   * on the cube face for the cube with the edge length 1
   * axx*x^2+ayy*y^2+axy*x*y+ax*x+ay*y+a
   */ 

  q3c_get_xy_minmax(axx, ayy, axy, ax, ay, a, &xmin, &xmax, &ymin, &ymax);
  

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
  
  for(face_count = 0; face_count <= multi_flag; out_nstack = 0, face_count++)
  {
    /* This the beginning of the mega-loop over multiple faces */
    
    if (face_count > 0)
      /* This "if" works when we pass through the secondary faces */ 
    {
      face_num = q3c_xy2facenum(2 * points[2 * (face_count - 1)],
                    2 * points[2 * (face_count - 1) + 1], face_num0);
      q3c_get_poly_coefs(face_num, ra0, dec0, rad, &axx, &ayy, &axy, &ax, &ay, &a);
      
      q3c_get_xy_minmax(axx, ayy, axy, ax, ay, a, &xmin, &xmax, &ymin, &ymax);
      
      xmax = (xmax > q3c_HALF ? q3c_HALF : xmax);
      xmin = (xmin < -q3c_HALF ? -q3c_HALF : xmin);
      ymax = (ymax > q3c_HALF ? q3c_HALF : ymax);
      ymin = (ymin < -q3c_HALF ? -q3c_HALF : ymin);
    }
    
#ifdef Q3C_DEBUG
    fprintf(stdout,"FACE RUN: %d FACE_NUM: %d\n", face_count, face_num);
#endif
    
    xesize = xmax - xmin; 
    yesize = ymax - ymin;
    xesize = xesize > yesize ? xesize : yesize;

    if (xesize * nside < 1) 
    /* If the region is too small */
    {
      xesize=1 / (q3c_coord_t)nside;
    }
    
    n0 = 1 << ((q3c_ipix_t)(-q3c_ceil((q3c_log(xesize) / q3c_log(2)))));  
    /* n0 is now the level of quadtree for which the minimal 
     * element is >~ our ellipse
     */

    cur_size = ((q3c_coord_t)1) / n0;
    //fprintf(stdout, "%lld %Lf %Lf %Lf\n", n0,cur_size,xesize,yesize);
#ifdef Q3C_DEBUG
    fprintf(stdout,"XMIN: "Q3C_COORD_FMT" XMAX: "Q3C_COORD_FMT" YMIN: "Q3C_COORD_FMT" YMAX: "Q3C_COORD_FMT"\n", xmin, xmax, ymin, ymax);
#endif

    /* Here we set up the stack with initial squares */
    
    ixmin = (q3c_HALF + xmin) * n0; /* Here I use the C truncation of floats */
    ixmax = (q3c_HALF + xmax) * n0; /* to integers */
    iymin = (q3c_HALF + ymin) * n0;
    iymax = (q3c_HALF + ymax) * n0;
    
    
    ixmax = (ixmax == n0 ? (n0 - 1) : ixmax);
    iymax = (iymax == n0 ? (n0 - 1) : iymax);
    
    
    cur_square = work_stack;
    if (ixmin == ixmax) 
    {
      if (iymin == iymax)
      {
        cur_square->x0 = ixmin;
        cur_square->y0 = iymin;
        cur_square->nside0 = n0;
        work_nstack = 1;
      }
      else
      {
        cur_square->x0 = ixmin;
        cur_square->y0 = iymin;
        cur_square->nside0 = n0;
        cur_square++;
        cur_square->x0 = ixmin;
        cur_square->y0 = iymax;
        cur_square->nside0 = n0; 
        work_nstack = 2;
      }
    }
    else
    {
      if (iymin == iymax)
      {
        cur_square->x0 = ixmin;
        cur_square->y0 = iymin;
        cur_square->nside0 = n0;
        cur_square++;
        cur_square->x0 = ixmax;
        cur_square->y0 = iymin;
        cur_square->nside0 = n0;      
        work_nstack = 2;
      }
      else
      {
        cur_square->x0 = ixmin;
        cur_square->y0 = iymin;
        cur_square->nside0 = n0;
        cur_square++;
        cur_square->x0 = ixmin;
        cur_square->y0 = iymax;
        cur_square->nside0 = n0; 
        cur_square++;
        cur_square->x0 = ixmax;
        cur_square->y0 = iymin;
        cur_square->nside0 = n0;
        cur_square++;
        cur_square->x0 = ixmax;
        cur_square->y0 = iymax;
        cur_square->nside0 = n0;
        work_nstack = 4;
      }
    }
    
    
    /* For this case the maximal increase of resolution of 2^res_depth
     * for each axis 
     */
    
    res_depth = nside / n0;
    /* If the the query is too small we cannot go up to max_depth since we
     * are limited by nside depth
     */
    res_depth = max_depth > res_depth ? res_depth : max_depth;

    for(i = 1; i <= res_depth; i++)
    {
#ifdef Q3C_DEBUG
      fprintf(stdout,"SQUARE RUN %d :\n", i);
#endif
#ifdef Q3C_DEBUG
    fprintf(stdout,"1) NUM squares in the stack %d\n",work_nstack);
#endif
      
      /* This loop perform the testing of all squares in work_stack */
      for(j = 0; j < work_nstack; j++)
      {
        cur_square = work_stack + j;
        cur_size=((q3c_coord_t) 1) / (cur_square->nside0);
        xc_cur = (( (q3c_coord_t) cur_square->x0) + q3c_HALF) / cur_square->nside0 - q3c_HALF;
        yc_cur = (( (q3c_coord_t) cur_square->y0) + q3c_HALF) / cur_square->nside0 - q3c_HALF;
        /* xc_cur and yc_cur -- center of the square (in the coordinate system
         * of big square [-0.5:0.5]x[-0.5:0.5]
         */
        //fprintf(stdout,"%Lf %Lf %Lf %d\n",xc_cur,yc_cur,cur_size, status);
        cur_square->status = q3c_circle_cover_check(xc_cur, yc_cur, cur_size,
                                                    xmin, xmax, ymin, ymax, 
                                                    axx, axy, ayy, ax, ay, a);

      }
      
#ifdef Q3C_DEBUG
      fprintf(stdout,"2) NUM squares in the stack %d\n",work_nstack);
#endif
      /* Now we select the fully covered set of squares from stack and put them
       * into out_stack, the partly covered squares are expanded to corresponding
       * set of 4 squares each.
       * explanation of the following scheme
       *    |xxxxxxxxxxxxx     xxxxxxxxxx|
       *                                 ^
       *                  /---/           tmp_stack2
       *                tmp_stack1
       */ 
      for(j = 0, tmp_stack1 = 0, tmp_stack2 = work_nstack; j < work_nstack; j++)
      {
        cur_square = work_stack + j;
        //fprintf(stdout,"%d %d %d\n",work_nstack,tmp_stack1,tmp_stack2);
        if (cur_square->status == q3c_PARTIAL)
        /* If this square partially intersects with the ellipse
         * I should split this square farther
         */
        {

          /* If this is the last stage of resolution loop, I will not split
           * the "partial" boxes
           */
          if (i == res_depth) continue; 
          tmp_stack1++;

          xtmp = 2 * cur_square->x0;
          ytmp = 2 * cur_square->y0;
          ntmp = 2 * cur_square->nside0;
          
          /* First I try to put the childrens of this square in the part of
           * the stack freed by trown away squares (which were disjunct from
           * the ellipse or which were fully covered by the ellipse)
           */ 
          for(k = 0; (k <= 3) && (tmp_stack1 > 0); k++)
          {
            cur_square = work_stack + (j + 1 - tmp_stack1);
            cur_square->x0 = xtmp + (k & 1);
            cur_square->y0 = ytmp + ((k & 2) >> 1);
            cur_square->nside0 = ntmp;
            tmp_stack1--;
          }
          
          for (; k <= 3; k++)
          {
            cur_square = work_stack + tmp_stack2;
            cur_square->x0 = xtmp + (k & 1);
            cur_square->y0 = ytmp + ((k & 2) >> 1);
            cur_square->nside0 = ntmp;
            tmp_stack2++;
          }
          
        }
        else 
        {
          if (cur_square->status == q3c_COVER)
          /* I put this square in the output list and 
           * free one place in the stack
           */
          {
            out_stack[out_nstack++] = *cur_square;
            tmp_stack1++;
          }
          else 
          /* This branch can be reached only if status==q3c_DISJUNCT */
          {
            tmp_stack1++;
            /* I just drop this square and free the place in the stack */
          }
        } 
      
      } /* end of updating of the list of squares loop */ 
      
#ifdef Q3C_DEBUG
      fprintf(stdout,"STACK STATE nw_stack: %d nt_stack1: %d nt_stack2: %d\n", work_nstack, tmp_stack1, tmp_stack2);
#endif
      
      
      if (i == res_depth) break; 
      /* After updating the list of squares I compute how much I have them now 
       * (except for the case of last resolution step) 
       */
      if (tmp_stack1 == 0) 
      {
        work_nstack = tmp_stack2;
      } 
      else 
      {
        if ((tmp_stack2-work_nstack) > tmp_stack1)
          {
          memcpy(work_stack + (work_nstack - tmp_stack1), work_stack + (tmp_stack2 - tmp_stack1), tmp_stack1 * sizeof(struct q3c_square)); 
          work_nstack = tmp_stack2 - tmp_stack1;
          }
        else
          {
          memcpy(work_stack + (work_nstack - tmp_stack1), work_stack + (tmp_stack2 - tmp_stack1), (tmp_stack2 - work_nstack) * sizeof(struct q3c_square));
          work_nstack = tmp_stack2 - tmp_stack1;
          }
      }
      
    } /* end of resolution loop */
    
    //   Old printing of the results
    
    for(i = 0; i < out_nstack; i++)
    {
      cur_square = out_stack + i;
#ifdef Q3C_DEBUG
      fprintf(stdout, "OUT: %f %f %d %d\n", cur_square->x0+0.5,cur_square->y0+0.5,cur_square->nside0,cur_square->status);
#endif
    }
    //fprintf(stdout,"XXX%d %d %d\n",work_nstack, tmp_stack1, tmp_stack2);
    
    for(i = 0; i < work_nstack; i++)
    {
      cur_square = work_stack + i;
#ifdef Q3C_DEBUG
      if (cur_square->status == q3c_PARTIAL)
        fprintf(stdout, "OUT1: %f %f %d %d\n", cur_square->x0+0.5,cur_square->y0+0.5,cur_square->nside0,cur_square->status);
#endif
    }
    
    
    
    ntmp = ((q3c_ipix_t) face_num) * nside * nside; 
    i1 = 1 << q3c_interleaved_nbits;
    
    /* Run through fully covered squares (we take them from out_stack) */ 
    for(i = 0; i < out_nstack; i++)
    {
      cur_square = out_stack + i;  
      ntmp1 = (nside / cur_square->nside0);  
      //fprintf(stdout, "XX%lld\n", ntmp1);
      xi = cur_square->x0 * ntmp1;
      yi = cur_square->y0 * ntmp1;
      

      /* Here we compute the ipix value for the bottom lower corner of the square */
  #ifdef Q3C_INT4 
      {
        ipix_tmp1 = ntmp + xbits[xi % i1] + ybits[yi % i1];
        /*4byte computation*/
      }
  #endif /* Q3C_INT4 */
  #ifdef Q3C_INT8
      {
        ipix_tmp1 = ntmp + xbits[xi % i1] + ybits[yi % i1] +
        (xbits[(xi >> q3c_interleaved_nbits) % i1] + ybits[(yi >> q3c_interleaved_nbits) % i1]) * i1 * i1;
        /*8byte computation*/
      }
  #endif /* Q3C_INT8 */
      
      ipix_tmp2=ipix_tmp1+(ntmp1*ntmp1);


      
      /* Now we have in ipix_tmp1 and ipix_tmp2 -- the pixel range for the 
       * query of current square 
       * The query should be     ipix_tmp1 =< II < ipix_tmp2  
       */
      
      out_ipix_arr_fulls[out_ipix_arr_fulls_pos++] = ipix_tmp1;
      out_ipix_arr_fulls[out_ipix_arr_fulls_pos++] = ipix_tmp2;
    
    } /* End of output run through fully covered squares */
    
    if (out_nstack == 0) 
    /* If the list of fully covered squares is empty */
    {
      /* Now we just do nothing  -- the stack of ipix'es will be just empty */
    }


    /* Run through partly covered squares (we take them from work_stack where
     * the cur_square->status == q3c_PARTIAL) 
     */ 
    for(i = 0, j = -1; i < work_nstack; i++)
    {
      cur_square = work_stack + i;
      if (cur_square->status!=q3c_PARTIAL) 
        continue;
      else 
        j+=1;  
      ntmp1 = (nside / cur_square->nside0);  
      //fprintf(stdout, "XX%lld\n", ntmp1);
      xi = cur_square->x0 * ntmp1;
      yi = cur_square->y0 * ntmp1;
      

      /* Here we compute the ipix value for the bottom lower corner of the square */
  #ifdef Q3C_INT4 
      {
        ipix_tmp1 = ntmp + xbits[xi % i1] + ybits[yi % i1];
        /*4byte computation*/
      }
  #endif /* Q3C_INT4 */
  #ifdef Q3C_INT8
      {
        ipix_tmp1 = ntmp + xbits[xi % i1] + ybits[yi % i1] +
        (xbits[(xi >> q3c_interleaved_nbits) % i1] + ybits[(yi >> q3c_interleaved_nbits) % i1]) * i1 * i1;
        /*8byte computation*/
      }
  #endif /* Q3C_INT8 */
      
      ipix_tmp2 = ipix_tmp1 + (ntmp1 * ntmp1);


      
      /* Now we have in ipix_tmp1 and ipix_tmp2 -- the pixel range for the 
       * query of current square 
       * The query should be     ipix_tmp1 =< II < ipix_tmp2  
       */

      out_ipix_arr_partials[out_ipix_arr_partials_pos++] = ipix_tmp1;
      out_ipix_arr_partials[out_ipix_arr_partials_pos++] = ipix_tmp2;
      
    
    } /* End of output run through partly covered squares */

  } /* End of the mega-loop over the faces */


  /* Now we should fill the tail of the out_ipix_arr_fulls stack by 
   * [1,-1] pairs  since our SQL code wants the arrays of fixed length
   */
   for(i = out_ipix_arr_fulls_pos; i < out_ipix_arr_fulls_length;)
   {
//     fprintf(stderr,"F%d\n",i);
     out_ipix_arr_fulls[i++] = 1;
     out_ipix_arr_fulls[i++] = -1;
   }


  /* Now we should fill the tail of the out_ipix_arr_fulls stack by 
   * [1,-1] pairs  since our SQL code wants the arrays of fixed length
   */
   for(i = out_ipix_arr_partials_pos; i < out_ipix_arr_partials_length;)
   {
//     fprintf(stderr,"P%d\n",i);
     out_ipix_arr_partials[i++] = 1;
     out_ipix_arr_partials[i++] = -1;
   }


  
#ifdef Q3C_DEBUG
//  fprintf(stderr, "COVER:%s\n", where_cover);
//  fprintf(stderr, "PARTLY:%s\n", where_part);
#endif

  

} /* End of q3c_new_radial_query() */



void q3c_poly_query(struct q3c_prm *hprm, struct q3c_poly *qp,
                          q3c_ipix_t *out_ipix_arr_fulls,
                          q3c_ipix_t *out_ipix_arr_partials)
{
  
  
  q3c_coord_t xmin, xmax, ymin, ymax, 
              xc_cur = 0 , yc_cur = 0, cur_size, xesize, yesize, xtmp, ytmp, 
              points[4];
                    
  q3c_ipix_t n0, nside = hprm->nside, ixmin, iymin, ixmax, iymax, ntmp,
              ntmp1, xi, yi, ipix_tmp1, ipix_tmp2, *xbits = hprm->xbits, 
              *ybits = hprm->ybits, i1;
  
  char face_num, multi_flag = 0, k, face_count, face_num0;
  int out_ipix_arr_fulls_pos = 0;
  int out_ipix_arr_partials_pos = 0;
  int out_ipix_arr_fulls_length = 100;//1600;
  int out_ipix_arr_partials_length = 100;//1600;
  /* !!!!!!!!IMPORTANT!!!! 
   * Keep in mind that those lengths are in fact multiplied by two 
   * maximal number of squares 
   */

  
  int work_nstack = 0, i, j, tmp_stack1, tmp_stack2, out_nstack = 0,
      res_depth;
  
  const int max_depth = 4; /* log2(Maximal increase of resolution) */
  
  struct q3c_square work_stack[11024], out_stack[11024], *cur_square;
  /* !!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!! 
   * Consider that the size of the stacks should directly depend on the
   * value of res_depth variable !
   * It seems that each of stacks should have the size 4*(2^(depth-1))
   */
  
  face_num = q3c_get_facenum_poly(qp);
  
  q3c_project_poly(qp, face_num);
  q3c_prepare_poly(qp);
  
  q3c_get_minmax_poly(qp, &xmin, &xmax, &ymin, &ymax);
  

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
  
  for(face_count = 0; face_count <= multi_flag; out_nstack = 0, face_count++)
  {
    /* This the beginning of the mega-loop over multiple faces */
    
    if (face_count > 0)
      /* This "if" works when we pass through the secondary faces */ 
    {
      face_num = q3c_xy2facenum(2 * points[2 * (face_count - 1)],
                    2 * points[2 * (face_count - 1) + 1], face_num0);

      q3c_project_poly(qp, face_num);
      q3c_prepare_poly(qp);
      
      q3c_get_minmax_poly(qp, &xmin, &xmax, &ymin, &ymax);
      
      xmax = (xmax > q3c_HALF ? q3c_HALF : xmax);
      xmin = (xmin < -q3c_HALF ? -q3c_HALF : xmin);
      ymax = (ymax > q3c_HALF ? q3c_HALF : ymax);
      ymin = (ymin < -q3c_HALF ? -q3c_HALF : ymin);
    }
    
#ifdef Q3C_DEBUG
    fprintf(stdout,"FACE RUN: %d FACE_NUM: %d\n", face_count, face_num);
#endif
    
    xesize = xmax - xmin; 
    yesize = ymax - ymin;
    xesize = xesize > yesize ? xesize : yesize;

    if (xesize * nside < 1) 
    /* If the region is too small */
    {
      xesize=1 / (q3c_coord_t)nside;
    }
    
    n0 = 1 << ((q3c_ipix_t)(-q3c_ceil((q3c_log(xesize) / q3c_log(2)))));  
    /* n0 is now the level of quadtree for which the minimal 
     * element is >~ our ellipse
     */

    cur_size = ((q3c_coord_t)1) / n0;
    //fprintf(stdout, "%lld %Lf %Lf %Lf\n", n0,cur_size,xesize,yesize);
#ifdef Q3C_DEBUG
    fprintf(stdout,"XMIN: "Q3C_COORD_FMT" XMAX: "Q3C_COORD_FMT" YMIN: "Q3C_COORD_FMT" YMAX: "Q3C_COORD_FMT"\n", xmin, xmax, ymin, ymax);
#endif

    /* Here we set up the stack with initial squares */
    
    ixmin = (q3c_HALF + xmin) * n0; /* Here I use the C truncation of floats */
    ixmax = (q3c_HALF + xmax) * n0; /* to integers */
    iymin = (q3c_HALF + ymin) * n0;
    iymax = (q3c_HALF + ymax) * n0;
    
    
    ixmax = (ixmax == n0 ? (n0 - 1) : ixmax);
    iymax = (iymax == n0 ? (n0 - 1) : iymax);
    
    
    cur_square = work_stack;
    if (ixmin == ixmax) 
    {
      if (iymin == iymax)
      {
        cur_square->x0 = ixmin;
        cur_square->y0 = iymin;
        cur_square->nside0 = (int) n0;
        work_nstack = 1;
      }
      else
      {
        cur_square->x0 = ixmin;
        cur_square->y0 = iymin;
        cur_square->nside0 = (int) n0;
        cur_square++;
        cur_square->x0 = ixmin;
        cur_square->y0 = iymax;
        cur_square->nside0 = (int) n0; 
        work_nstack = 2;
      }
    }
    else
    {
      if (iymin == iymax)
      {
        cur_square->x0 = ixmin;
        cur_square->y0 = iymin;
        cur_square->nside0 = (int) n0;
        cur_square++;
        cur_square->x0 = ixmax;
        cur_square->y0 = iymin;
        cur_square->nside0 = (int) n0;      
        work_nstack = 2;
      }
      else
      {
        cur_square->x0 = ixmin;
        cur_square->y0 = iymin;
        cur_square->nside0 = (int) n0;
        cur_square++;
        cur_square->x0 = ixmin;
        cur_square->y0 = iymax;
        cur_square->nside0 = (int) n0; 
        cur_square++;
        cur_square->x0 = ixmax;
        cur_square->y0 = iymin;
        cur_square->nside0 = (int) n0;
        cur_square++;
        cur_square->x0 = ixmax;
        cur_square->y0 = iymax;
        cur_square->nside0 = n0;
        work_nstack = 4;
      }
    }
    
    
    /* For this case the maximal increase of resolution of 2^res_depth
     * for each axis 
     */
    
    res_depth = nside / n0;
    /* If the the query is too small we cannot go up to max_depth since we
     * are limited by nside depth
     */
    res_depth = max_depth > res_depth ? res_depth : max_depth;

    for(i = 1; i <= res_depth; i++)
    {
#ifdef Q3C_DEBUG
      fprintf(stdout,"SQUARE RUN %d :\n", i);
#endif
#ifdef Q3C_DEBUG
    fprintf(stdout,"1) NUM squares in the stack %d\n",work_nstack);
#endif
      
      /* This loop perform the testing of all squares in work_stack */
      for(j = 0; j < work_nstack; j++)
      {
        cur_square = work_stack + j;
        cur_size=((q3c_coord_t) 1) / (cur_square->nside0);
        xc_cur = (( (q3c_coord_t) cur_square->x0) + q3c_HALF) / cur_square->nside0 - q3c_HALF;
        yc_cur = (( (q3c_coord_t) cur_square->y0) + q3c_HALF) / cur_square->nside0 - q3c_HALF;
        /* xc_cur and yc_cur -- center of the square (in the coordinate system
         * of big square [-0.5:0.5]x[-0.5:0.5]
         */
        //fprintf(stdout,"%Lf %Lf %Lf %d\n",xc_cur,yc_cur,cur_size, status);
#ifdef Q3C_DEBUG 
        fprintf(stderr,"Trying (%f %f %f)(%f %f %f %f) --- ",xc_cur,yc_cur,cur_size,xc_cur-cur_size/2,xc_cur+cur_size/2,yc_cur-cur_size/2,yc_cur+cur_size/2);
#endif
        
        cur_square->status = q3c_poly_cover_check(qp, xc_cur, yc_cur,
                                                  cur_size);
#ifdef Q3C_DEBUG 
        fprintf(stderr,"%d\n",cur_square->status);
#endif


      }
      
#ifdef Q3C_DEBUG
      fprintf(stdout,"2) NUM squares in the stack %d\n",work_nstack);
#endif
      /* Now we select the fully covered set of squares from stack and put them
       * into out_stack, the partly covered squares are expanded to corresponding
       * set of 4 squares each.
       * explanation of the following scheme
       *    |xxxxxxxxxxxxx     xxxxxxxxxx|
       *                                 ^
       *                  /---/           tmp_stack2
       *                tmp_stack1
       */ 
      for(j = 0, tmp_stack1 = 0, tmp_stack2 = work_nstack; j < work_nstack; j++)
      {
        cur_square = work_stack + j;
        //fprintf(stdout,"%d %d %d\n",work_nstack,tmp_stack1,tmp_stack2);
        if (cur_square->status == q3c_PARTIAL)
        /* If this square partially intersects with the ellipse
         * I should split this square farther
         */
        {

          /* If this is the last stage of resolution loop, I will not split
           * the "partial" boxes
           */
          if (i == res_depth) continue; 
          tmp_stack1++;

          xtmp = 2 * cur_square->x0;
          ytmp = 2 * cur_square->y0;
          ntmp = 2 * cur_square->nside0;
          
          /* First I try to put the childrens of this square in the part of
           * the stack freed by trown away squares (which were disjunct from
           * the ellipse or which were fully covered by the ellipse)
           */ 
          for(k = 0; (k <= 3) && (tmp_stack1 > 0); k++)
          {
            cur_square = work_stack + (j + 1 - tmp_stack1);
            cur_square->x0 = xtmp + (k & 1);
            cur_square->y0 = ytmp + ((k & 2) >> 1);
            cur_square->nside0 = ntmp;
            tmp_stack1--;
          }
          
          for (; k <= 3; k++)
          {
            cur_square = work_stack + tmp_stack2;
            cur_square->x0 = xtmp + (k & 1);
            cur_square->y0 = ytmp + ((k & 2) >> 1);
            cur_square->nside0 = (int) ntmp;
            tmp_stack2++;
          }
          
        }
        else 
        {
          if (cur_square->status == q3c_COVER)
          /* I put this square in the output list and 
           * free one place in the stack
           */
          {
            out_stack[out_nstack++] = *cur_square;
            tmp_stack1++;
          }
          else 
          /* This branch can be reached only if status==q3c_DISJUNCT */
          {
            tmp_stack1++;
            /* I just drop this square and free the place in the stack */
          }
        } 
      
      } /* end of updating of the list of squares loop */ 
      
#ifdef Q3C_DEBUG
      fprintf(stdout,"STACK STATE nw_stack: %d nt_stack1: %d nt_stack2: %d\n", work_nstack, tmp_stack1, tmp_stack2);
#endif
      
      
      if (i == res_depth) break; 
      /* After updating the list of squares I compute how much I have them now 
       * (except for the case of last resolution step) 
       */
      if (tmp_stack1 == 0) 
      {
        work_nstack = tmp_stack2;
      } 
      else 
      {
        if ((tmp_stack2-work_nstack) > tmp_stack1)
          {
          memcpy(work_stack + (work_nstack - tmp_stack1), work_stack + (tmp_stack2 - tmp_stack1), tmp_stack1 * sizeof(struct q3c_square)); 
          work_nstack = tmp_stack2 - tmp_stack1;
          }
        else
          {
          memcpy(work_stack + (work_nstack - tmp_stack1), work_stack + (tmp_stack2 - tmp_stack1), (tmp_stack2 - work_nstack) * sizeof(struct q3c_square));
          work_nstack = tmp_stack2 - tmp_stack1;
          }
      }
      
    } /* end of resolution loop */
    
    //   Old printing of the results
    
    for(i = 0; i < out_nstack; i++)
    {
      cur_square = out_stack + i;
#ifdef Q3C_DEBUG
      fprintf(stdout, "OUT: %f %f %d %d\n", cur_square->x0+0.5,cur_square->y0+0.5,cur_square->nside0,cur_square->status);
#endif
    }
    //fprintf(stdout,"XXX%d %d %d\n",work_nstack, tmp_stack1, tmp_stack2);
    
    for(i = 0; i < work_nstack; i++)
    {
      cur_square = work_stack + i;
#ifdef Q3C_DEBUG
      if (cur_square->status == q3c_PARTIAL)
        fprintf(stdout, "OUT1: %f %f %d %d\n", cur_square->x0+0.5,cur_square->y0+0.5,cur_square->nside0,cur_square->status);
#endif
    }
    
    
    
    ntmp = ((q3c_ipix_t) face_num) * nside * nside; 
    i1 = 1 << q3c_interleaved_nbits;
    
    /* Run through fully covered squares (we take them from out_stack) */ 
    for(i = 0; i < out_nstack; i++)
    {
      cur_square = out_stack + i;  
      ntmp1 = (nside / cur_square->nside0);  
      //fprintf(stdout, "XX%lld\n", ntmp1);
      xi = cur_square->x0 * ntmp1;
      yi = cur_square->y0 * ntmp1;
      

      /* Here we compute the ipix value for the bottom lower corner of the square */
  #ifdef Q3C_INT4 
      {
        ipix_tmp1 = ntmp + xbits[xi % i1] + ybits[yi % i1];
        /*4byte computation*/
      }
  #endif /* Q3C_INT4 */
  #ifdef Q3C_INT8
      {
        ipix_tmp1 = ntmp + xbits[xi % i1] + ybits[yi % i1] +
        (xbits[(xi >> q3c_interleaved_nbits) % i1] + ybits[(yi >> q3c_interleaved_nbits) % i1]) * i1 * i1;
        /*8byte computation*/
      }
  #endif /* Q3C_INT8 */
      
      ipix_tmp2=ipix_tmp1+(ntmp1*ntmp1);


      
      /* Now we have in ipix_tmp1 and ipix_tmp2 -- the pixel range for the 
       * query of current square 
       * The query should be     ipix_tmp1 =< II < ipix_tmp2  
       */
      
      out_ipix_arr_fulls[out_ipix_arr_fulls_pos++] = ipix_tmp1;
      out_ipix_arr_fulls[out_ipix_arr_fulls_pos++] = ipix_tmp2;
    
    } /* End of output run through fully covered squares */
    
    if (out_nstack == 0) 
    /* If the list of fully covered squares is empty */
    {
      /* Now we just do nothing  -- the stack of ipix'es will be just empty */
    }


    /* Run through partly covered squares (we take them from work_stack where
     * the cur_square->status == q3c_PARTIAL) 
     */ 
    for(i = 0, j = -1; i < work_nstack; i++)
    {
      cur_square = work_stack + i;
      if (cur_square->status!=q3c_PARTIAL) 
        continue;
      else 
        j+=1;  
      ntmp1 = (nside / cur_square->nside0);  
      //fprintf(stdout, "XX%lld\n", ntmp1);
      xi = cur_square->x0 * ntmp1;
      yi = cur_square->y0 * ntmp1;
      

      /* Here we compute the ipix value for the bottom lower corner of the square */
  #ifdef Q3C_INT4 
      {
        ipix_tmp1 = ntmp + xbits[xi % i1] + ybits[yi % i1];
        /*4byte computation*/
      }
  #endif /* Q3C_INT4 */
  #ifdef Q3C_INT8
      {
        ipix_tmp1 = ntmp + xbits[xi % i1] + ybits[yi % i1] +
        (xbits[(xi >> q3c_interleaved_nbits) % i1] + ybits[(yi >> q3c_interleaved_nbits) % i1]) * i1 * i1;
        /*8byte computation*/
      }
  #endif /* Q3C_INT8 */
      
      ipix_tmp2 = ipix_tmp1 + (ntmp1 * ntmp1);


      
      /* Now we have in ipix_tmp1 and ipix_tmp2 -- the pixel range for the 
       * query of current square 
       * The query should be     ipix_tmp1 =< II < ipix_tmp2  
       */

      out_ipix_arr_partials[out_ipix_arr_partials_pos++] = ipix_tmp1;
      out_ipix_arr_partials[out_ipix_arr_partials_pos++] = ipix_tmp2;
      
    
    } /* End of output run through partly covered squares */

  } /* End of the mega-loop over the faces */


  /* Now we should fill the tail of the out_ipix_arr_fulls stack by 
   * [1,-1] pairs  since our SQL code wants the arrays of fixed length
   */
   for(i = out_ipix_arr_fulls_pos; i < out_ipix_arr_fulls_length;)
   {
//     fprintf(stderr,"F%d\n",i);
     out_ipix_arr_fulls[i++] = 1;
     out_ipix_arr_fulls[i++] = -1;
   }


  /* Now we should fill the tail of the out_ipix_arr_fulls stack by 
   * [1,-1] pairs  since our SQL code wants the arrays of fixed length
   */
   for(i = out_ipix_arr_partials_pos; i < out_ipix_arr_partials_length;)
   {
//     fprintf(stderr,"P%d\n",i);
     out_ipix_arr_partials[i++] = 1;
     out_ipix_arr_partials[i++] = -1;
   }


  
#ifdef Q3C_DEBUG
//  fprintf(stderr, "COVER:%s\n", where_cover);
//  fprintf(stderr, "PARTLY:%s\n", where_part);
#endif

  

} /* End of radial_query() */


