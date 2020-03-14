/*
       Copyright (C) 2004-2020 Sergey Koposov

    Email: skoposov@cmu.edu

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
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "my_bits.h"

static char __q3c_version[]=Q3C_VERSION;

static int q3c_setup_square_stack(struct q3c_square *stack, q3c_coord_t xmin,
                                  q3c_coord_t ymin, q3c_coord_t xmax, q3c_coord_t ymax,
                                  int n0);
static void q3c_stack_expand(struct q3c_square* work_stack, int *work_nstack,
                             struct q3c_square* out_stack, int *out_nstack,
                             int cur_depth, int res_depth);
static void q3c_output_stack( struct q3c_prm *hprm,
                              struct q3c_square *out_stack, int out_nstack,
                              struct q3c_square *work_stack, int work_nstack,
                              int face_num, int nside,
                              q3c_ipix_t *out_ipix_arr_fulls,
                              int *out_ipix_arr_fulls_pos,
                              q3c_ipix_t *out_ipix_arr_partials,
                              int *out_ipix_arr_partials_pos);


static void q3c_fast_get_equatorial_ellipse_xy_minmax(q3c_coord_t alpha,
                                                      q3c_coord_t delta,
                                                      q3c_coord_t d, q3c_coord_t e,
                                                      q3c_coord_t PA,
                                                      q3c_coord_t *ymin,
                                                      q3c_coord_t *ymax,
                                                      q3c_coord_t *zmin,
                                                      q3c_coord_t *zmax);

static void q3c_fast_get_equatorial_ellipse_xy_minmax_and_poly_coefs(q3c_coord_t alpha,
                                                                     q3c_coord_t delta,
                                                                     q3c_coord_t d, q3c_coord_t e,
                                                                     q3c_coord_t PA,
                                                                     q3c_coord_t *ymin,
                                                                     q3c_coord_t *ymax,
                                                                     q3c_coord_t *zmin,
                                                                     q3c_coord_t *zmax,
                                                                     q3c_coord_t *ayy,
                                                                     q3c_coord_t *azz,
                                                                     q3c_coord_t *ayz,
                                                                     q3c_coord_t *ay,
                                                                     q3c_coord_t *az,
                                                                     q3c_coord_t *a);

static void q3c_fast_get_polar_ellipse_xy_minmax_and_poly_coefs(q3c_coord_t alpha,
                                                                q3c_coord_t delta,
                                                                q3c_coord_t d, q3c_coord_t e,
                                                                q3c_coord_t PA,
                                                                q3c_coord_t *ymin,
                                                                q3c_coord_t *ymax,
                                                                q3c_coord_t *zmin,
                                                                q3c_coord_t *zmax,
                                                                q3c_coord_t *ayy,
                                                                q3c_coord_t *azz,
                                                                q3c_coord_t *ayz,
                                                                q3c_coord_t *ay,
                                                                q3c_coord_t *az,
                                                                q3c_coord_t *a);
static void q3c_fast_get_ellipse_xy_minmax_and_poly_coefs(char face_num,
                                                          q3c_coord_t ra0,
                                                          q3c_coord_t dec0,
                                                          q3c_coord_t d0,
                                                          q3c_coord_t e,
                                                          q3c_coord_t PA0,
                                                          q3c_coord_t *ymin,
                                                          q3c_coord_t *ymax,
                                                          q3c_coord_t *zmin,
                                                          q3c_coord_t *zmax,
                                                          q3c_coord_t *ayy,
                                                          q3c_coord_t *azz,
                                                          q3c_coord_t *ayz,
                                                          q3c_coord_t *ay,
                                                          q3c_coord_t *az,
                                                          q3c_coord_t *a);

static void q3c_fast_get_polar_ellipse_xy_minmax(q3c_coord_t alpha,
                                                 q3c_coord_t delta, q3c_coord_t d,
                                                 q3c_coord_t e, q3c_coord_t PA,
                                                 q3c_coord_t *ymin,
                                                 q3c_coord_t *ymax,
                                                 q3c_coord_t *zmin,
                                                 q3c_coord_t *zmax);

static void q3c_fast_get_xy_minmax(char, q3c_region, void *, q3c_coord_t *,
                                   q3c_coord_t *, q3c_coord_t *,
                                   q3c_coord_t *);

static void q3c_fast_get_circle_xy_minmax(char, q3c_coord_t, q3c_coord_t, q3c_coord_t,
                                          q3c_coord_t *, q3c_coord_t *, q3c_coord_t *,
                                          q3c_coord_t *);

static void q3c_fast_get_ellipse_xy_minmax(char, q3c_coord_t, q3c_coord_t,
                                           q3c_coord_t,q3c_coord_t, q3c_coord_t,
                                           q3c_coord_t *, q3c_coord_t *, q3c_coord_t *,
                                           q3c_coord_t *);

static void array_filler(q3c_ipix_t *fulls, int fullpos,
                         q3c_ipix_t *parts, int partpos);

void q3c_get_version(char *out, int maxchar)
{
	strncpy(out,__q3c_version,maxchar);
}


/* Distance calculation routine, inputs and outputs are in degrees */
q3c_coord_t q3c_dist(q3c_coord_t ra1, q3c_coord_t dec1,
                     q3c_coord_t ra2, q3c_coord_t dec2)
{
	/* Fast and precise way to compute the distance on the sphere
	 * it uses just 3 evaluations of trigonometric functions
	 */

	q3c_coord_t x, y, z;
	x = q3c_sin ((ra1 - ra2) / 2 * Q3C_DEGRA );
	x *= x;
	y = q3c_sin ((dec1 - dec2) / 2 * Q3C_DEGRA);
	y *= y;

	/* Seem to be more precise :) */
	z = q3c_cos ((dec1 + dec2)/2 * Q3C_DEGRA);
	z*=z;

	return 2 * q3c_asin (q3c_sqrt (x * (z - y) + y)) * Q3C_RADEG;
}

/* sin(Distance) calculation routine, inputs and outputs are in degrees */
q3c_coord_t q3c_sindist(q3c_coord_t ra1, q3c_coord_t dec1,
                        q3c_coord_t ra2, q3c_coord_t dec2)
{
	/* Fast and precise way to compute the distance on the sphere
	 * it uses just 3 evaluations of trigonometric functions
	 */
	q3c_coord_t x, y, z;
	x = q3c_sin ((ra1 - ra2) / 2 * Q3C_DEGRA);
	x *= x;
	y = q3c_sin ((dec1 - dec2) / 2 * Q3C_DEGRA);
	y *= y;

	/* Seem to be more precise :) */
	z = q3c_cos ((dec1 + dec2)/2 * Q3C_DEGRA);
	z*=z;

	return x * (z - y) + y;
}



/* convert angular coordinates (ra,dec) -> ipix
 * ang2ipix is also outputting x,y on the cube face
 * Coordinates on the cube face are x[-0.5,0.5] y[-0.5,0.5]
 * ipix structure is the following:
 * Bit 0(leftmost) : unused
 * Bit 1-3: Cube face id (0 is the top face, 5 is  the bottom face,
 * 1-4 are faces looking towards (x,y)=(1,0), (0,1), (-1,0), (0,-1)
 * Bit 4-63: 60-bit long location in the quadtree on the cube face,
 * encoded by z-order (e.g. two interleaved bit strings on for x,
 * another for y)
 * The mapping between x,y,z to (ra,dec) is such that
 *  (x,y,z)=(1,0,0) corresponds to (ra,dec)=(0,0)
 *  (x,y,z)=(0,0,1) corresponds to (ra,dec)=(0,90)
 */
void q3c_ang2ipix_xy (struct q3c_prm *hprm, q3c_coord_t ra0, q3c_coord_t dec0,
                      char *out_face_num, q3c_ipix_t *ipix, q3c_coord_t *x_out,
                      q3c_coord_t *y_out)
/* ra in degrees, dec in degrees       */
/* strictly 0<=ra<360 and -90<=dec<=90 */
{
	q3c_coord_t x0 = 0, y0 = 0, ra1, dec1, tmp0, td1;
	q3c_coord_t ra,dec;
	q3c_ipix_t nside = hprm->nside, *xbits = hprm->xbits,
	           *ybits = hprm->ybits, xi, yi;
	char face_num;

	/* We check against crazy right ascensions */
	ra = UNWRAP_RA(ra0);

	/* protection against wrong declinations */
	if (dec0 > 90)
	{
		dec = 90;
	}
	else if (dec0 < -90)
	{
		dec = -90;
	}
	else
	{
		dec = dec0;
	}

	face_num = q3c_fmod ((ra + 45) / 90, 4);
	/* for equatorial pixels we'll have face_num from 1 to 4 */
	ra1 = Q3C_DEGRA * (ra - 90 * (q3c_coord_t)face_num);
	dec1 = Q3C_DEGRA * dec;
	x0 = q3c_tan (ra1);
	td1 = q3c_tan(dec1);
	y0 = td1 / q3c_cos (ra1);
	face_num++;

	if (y0 > 1)
	{
		face_num = 0;
		ra1 = Q3C_DEGRA * ra;
		tmp0 = 1 / td1;
		q3c_sincos (ra1, x0, y0);

		x0 *= tmp0;
		y0 *= (-tmp0);
		/*x0 = q3c_sin(ra1) / q3c_tan(dec1);*/
		/*y0 = -q3c_cos(ra1) / q3c_tan(dec1);*/
		/* I don't know
		 * Probably I should write (sin(ra)/sin(dec))*cos(dec) to
		 * not loose the precision in the region where dec ~=90deg
		 */
	}
	else if (y0 < -1)
	{
		face_num = 5;
		ra1 = Q3C_DEGRA * ra;
		tmp0 = 1 / td1;
		q3c_sincos (ra1, x0, y0);

		x0 *= (-tmp0);
		y0 *= (-tmp0);
		/*x0 = -q3c_sin(ra1) / q3c_tan(dec1);*/
		/*y0 = -q3c_cos(ra1) / q3c_tan(dec1);*/
	}

	*x_out = x0 / 2;
	*y_out = y0 / 2;

	x0 = (x0 + 1) / 2;
	y0 = (y0 + 1) / 2;

	/* Now I produce the final pixel value by converting x and y values
	 * to bitfields and combining them by interleaving, using the
	 * predefined arrays xbits and ybits
	 */

	xi = (q3c_ipix_t)(x0 * nside);
	yi = (q3c_ipix_t)(y0 * nside);

	/* This two following statements are written to handle the
	 * case of upper right corner of base square */
	if (xi == nside)
	{
		xi--;
	}
	if (yi == nside)
	{
		yi--;
	}

	*ipix = q3c_xiyi2ipix(nside, xbits, ybits, face_num, xi, yi);

	*out_face_num = face_num;

}

/* convert coordinates (ra,dec) -> ipix
 * ra, dec in degrees
 * and strictly 0<=ra<360 and -90<=dec<=90
 */
void q3c_ang2ipix(struct q3c_prm *hprm, q3c_coord_t ra0, q3c_coord_t dec0,
                  q3c_ipix_t *ipix)
{
	q3c_coord_t tmpx, tmpy;
	char face;
	q3c_ang2ipix_xy(hprm, ra0, dec0, &face, ipix, &tmpx, &tmpy);
}

/* get the cube face number for a given coordinates
 * ra, dec in degrees
 * and strictly 0<=ra<360 and -90<=dec<=90
 */
char q3c_get_facenum(q3c_coord_t ra, q3c_coord_t dec)
{
	q3c_coord_t y0 = 0;
	char face_num;

	if (dec >= 90)
	/* Poles */
	{
		return 0;
	}
	else if (dec <= -90)
	{
		return 5;
	}

	face_num = q3c_fmod ((ra + 45) / 90, 4);
	/*for equatorial pixels we'll have face_num from 1 to 4 */

	y0 = q3c_tan(dec * Q3C_DEGRA) /
	     q3c_cos(Q3C_DEGRA * (ra - 90 * (q3c_coord_t)face_num));

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


/* get the main cube face number for a given region
 * CIRCLE/ELLIPSE/POLYGON
 */
char q3c_get_region_facenum(q3c_region region, void *data)
{
	switch(region)
	{
	case Q3C_CIRCLE:
	{
		q3c_circle_region circle = *(q3c_circle_region*)data;
		return q3c_get_facenum(circle.ra, circle.dec);
	}
	case Q3C_ELLIPSE:
	{
		q3c_ellipse_region ellipse = *(q3c_ellipse_region*) data;
		return q3c_get_facenum(ellipse.ra, ellipse.dec);
	}
	case Q3C_POLYGON:
	{
		q3c_poly poly = *(q3c_poly *) data;
		return q3c_get_facenum_poly(&poly);
	}
	default:
		return 1;
	}
}


/* Check that the given point (alpha, delta0)
 * is within the ellipse specified by
 * center, maj_ax, axis ratio and positional angle
 */
char q3c_in_ellipse(q3c_coord_t alpha, q3c_coord_t delta0,
                    q3c_coord_t alpha1, q3c_coord_t delta01, q3c_coord_t d0,
                    q3c_coord_t e, q3c_coord_t PA0)
{
	q3c_coord_t d_alpha = (alpha1 - alpha) * Q3C_DEGRA;
	q3c_coord_t delta1 = delta01 * Q3C_DEGRA;
	q3c_coord_t delta = delta0 * Q3C_DEGRA;
	q3c_coord_t PA = PA0 * Q3C_DEGRA;
	q3c_coord_t d = d0 * Q3C_DEGRA;

	q3c_coord_t t1 = q3c_cos(d_alpha);
	q3c_coord_t t22 = q3c_sin(d_alpha);
	q3c_coord_t t3 = q3c_cos(delta1);
	q3c_coord_t t32 = q3c_sin(delta1);
	q3c_coord_t t6 = q3c_cos(delta);
	q3c_coord_t t26 = q3c_sin(delta);
	q3c_coord_t t9 = q3c_cos(d);
	q3c_coord_t t55 = q3c_sin(d);

	q3c_coord_t t2;
	q3c_coord_t t4;
	q3c_coord_t t5;
	q3c_coord_t t7;
	q3c_coord_t t8;
	q3c_coord_t t10;
	q3c_coord_t t11;
	q3c_coord_t t13;
	q3c_coord_t t14;
	q3c_coord_t t15;
	q3c_coord_t t18;
	q3c_coord_t t19;
	q3c_coord_t t24;
	q3c_coord_t t31;
	q3c_coord_t t36;
	q3c_coord_t t37;
	q3c_coord_t t38;
	q3c_coord_t t45;

	q3c_coord_t t56;
	q3c_coord_t t57;
	q3c_coord_t t60;
	q3c_coord_t t61;
	q3c_coord_t t63;

	if ((t3 * t6 * t1 + t32 * t26) < 0)
	{
		return 0;
	}

	t2 = t1*t1;

	t4 = t3*t3;
	t5 = t2*t4;

	t7 = t6*t6;
	t8 = t5*t7;

	t10 = t9*t9;
	t11 = t7*t10;
	t13 = q3c_cos(PA);
	t14 = t13*t13;
	t15 = t14*t10;
	t18 = t7*t14;
	t19 = t18*t10;

	t24 = q3c_sin(PA);

	t31 = t1*t3;

	t36 = 2.0*t31*t32*t26*t6;
	t37 = t31*t32;
	t38 = t26*t6;
	t45 = t4*t10;

	t56 = t55*t55;
	t57 = t4*t7;
	t60 = -t8+t5*t11+2.0*t5*t15-t5*t19-2.0*t1*t4*t22*t10*t24*t13*t26-t36+2.0*t37*t38*t10-2.0*t37*t38*t15-t45*t14-t45*t2+2.0*t22*t3*t32*t6*t24*t10*t13-t56+t7-t11+t4-t57+t57*t10+t19-t18*t45;
	t61 = e*e;
	t63 = t60*t61+t8+t57-t4-t7+t56+t36;
	return t63 > 0;
}


/* Checking whether the box (xmin,ymin,xmax,ymax) intersects other faces or
 * not. If yes, I setup the array "points" designed to help us work on
 * other faces ( points array will then have the coordinates on a main face
 * which should be mapped to other faces
 * !!!!!!!!!!!!!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!!!! It does change the arguments (xmin,xmax, ymin, ymax) !!!!!!!!
 */
void q3c_multi_face_check(q3c_coord_t *xmin0, q3c_coord_t *ymin0,
                          q3c_coord_t *xmax0, q3c_coord_t *ymax0,
                          q3c_coord_t *points, char *multi_flag)
{
	const q3c_coord_t xmin = *xmin0,
	                  xmax = *xmax0,
	                  ymin = *ymin0,
	                  ymax = *ymax0;
	if (xmin < -Q3C_HALF)
	{
		if (ymin < -Q3C_HALF)
		{
			points[0] = xmax;
			points[1] = ymin;
			points[2] = xmin;
			points[3] = ymax;
			*multi_flag = 2;
			*xmin0 = -Q3C_HALF;
			*ymin0 = -Q3C_HALF;
		}
		else
		{
			if (ymax > Q3C_HALF)
			{
				points[0] = xmax;
				points[1] = ymax;
				points[2] = xmin;
				points[3] = ymin;
				*multi_flag = 2;
				*xmin0 = -Q3C_HALF;
				*ymax0 = Q3C_HALF;
			}
			else
			{
				points[0] = xmin;
				points[1] = (ymin + ymax) / 2;
				*multi_flag = 1;
				*xmin0 = -Q3C_HALF;
			}
		}
	}
	else
	{
		if (xmax > Q3C_HALF)
		{
			if (ymin < -Q3C_HALF)
			{
				points[0] = xmin;
				points[1] = ymin;
				points[2] = xmax;
				points[3] = ymax;
				*multi_flag = 2;
				*xmax0 = Q3C_HALF;
				*ymin0 = -Q3C_HALF;
			}
			else
			{
				if (ymax > Q3C_HALF)
				{
					points[0] = xmin;
					points[1] = ymax;
					points[2] = xmax;
					points[3] = ymin;
					*multi_flag = 2;
					*xmax0 = Q3C_HALF;
					*ymax0 = Q3C_HALF;
				}
				else
				{
					points[0] = xmax;
					points[1] = (ymin + ymax) / 2;
					*multi_flag = 1;
					*xmax0 = Q3C_HALF;
				}
			}
		}
		else
		{
			if (ymin < -Q3C_HALF)
			{
				points[0] = (xmin + xmax) / 2;
				points[1] = ymin;
				*multi_flag = 1;
				*ymin0 = -Q3C_HALF;
			}
			else
			{
				if (ymax > Q3C_HALF)
				{
					points[0] = (xmin + xmax) / 2;
					points[1] = ymax;
					*multi_flag = 1;
					*ymax0 = Q3C_HALF;
				}
				else
				{
					*multi_flag = 0;
				}
			}
		}
	}
}




/* Get the list of 4 ipix ranges
 * which decsribe the neighborhood of a given point
 * specified by q3c_region
 * ra in degrees, dec in degrees, radius in degrees
 * strictly 0<=ra<360 and -90<=dec<=90
 */
void q3c_get_nearby(struct q3c_prm *hprm, q3c_region region, void *region_data,
                    q3c_ipix_t *ipix)
{
	q3c_coord_t xmin, xmax, ymin, ymax, xesize, yesize, points[4];
	q3c_ipix_t nside = hprm->nside, *xbits = hprm->xbits, *ybits = hprm->ybits,
	           *ipix_cur = ipix, ipix0, xi, yi, n0, n1, ixmin,
	           ixmax, iymin, iymax, xistack[4], yistack[4], facestack[4],
	           nstack[4];
	char face_num, face_num0, multi_flag;
	int i, nistack = 0;
	const q3c_coord_t q3c_lg2 = Q3C_LG2;

	if (q3c_too_big_check(region, region_data))
	{
		/* the whole sky */
		q3c_ipix_t maxval = 6*(nside*nside);
		*(ipix_cur++) = -1;
		*(ipix_cur++) = maxval;
		for(i = 1; i < 4; i++ )
		{
			*(ipix_cur++) = 1;
			*(ipix_cur++) = -1;
		}
		return;
	}

	face_num = q3c_get_region_facenum(region, region_data);
	face_num0 = face_num;

	q3c_fast_get_xy_minmax(face_num, region, region_data, &xmin, &xmax, &ymin, &ymax);
	/* xmin, xmax, ymin, ymax are in the coordinate system of the cube face
	 * where -0.5<=x<=0.5 and -0.5<=y<=0.5
	 */
#ifdef Q3C_DEBUG
	fprintf(stderr, "XMIN: %f XMAX: %f YMIN: %f YMAX: %f\n", xmin, xmax, ymin, ymax);
#endif

	q3c_multi_face_check(&xmin, &ymin, &xmax, &ymax, points, &multi_flag);

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

		ixmin = (Q3C_HALF + xmin) * n0;
		ixmax = (Q3C_HALF + xmax) * n0;
		iymin = (Q3C_HALF + ymin) * n0;
		iymax = (Q3C_HALF + ymax) * n0;

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

			ixmin = (Q3C_HALF + xmin) * n0;
			ixmax = (Q3C_HALF + xmax) * n0;
			iymin = (Q3C_HALF + ymin) * n0;
			iymax = (Q3C_HALF + ymax) * n0;

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
					nstack[1] = n1;
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
			q3c_fast_get_xy_minmax(face_num, region, region_data, &xmin,
			                       &xmax, &ymin, &ymax);

			xmax = (xmax > Q3C_HALF ? Q3C_HALF : xmax);
			xmin = (xmin < -Q3C_HALF ? -Q3C_HALF : xmin);
			ymax = (ymax > Q3C_HALF ? Q3C_HALF : ymax);
			ymin = (ymin < -Q3C_HALF ? -Q3C_HALF : ymin);
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

			ixmin = (Q3C_HALF + xmin) * n0;
			ixmax = (Q3C_HALF + xmax) * n0;
			iymin = (Q3C_HALF + ymin) * n0;
			iymax = (Q3C_HALF + ymax) * n0;

			ixmax = (ixmax == n0 ? n0 - 1 : ixmax);
			iymax = (iymax == n0 ? n0 - 1 : iymax);

			n1 = nside / n0;

			if (ixmin == ixmax)
			{
				if (iymin == iymax)
				{
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

			ixmin = (Q3C_HALF + xmin) * n0;
			ixmax = (Q3C_HALF + xmax) * n0;
			iymin = (Q3C_HALF + ymin) * n0;
			iymax = (Q3C_HALF + ymax) * n0;

			ixmax = (ixmax == n0 ? n0 - 1 : ixmax);
			iymax = (iymax == n0 ? n0 - 1 : iymax);

			n1 = nside / n0;

			xistack[0] = (q3c_ipix_t)(ixmin * n1);
			yistack[0] = (q3c_ipix_t)(iymin * n1);
			facestack[0] = face_num;
			nstack[0] = n1;
			nistack = 1;

			face_num = q3c_xy2facenum(2 * points[0], 2 * points[1], face_num0);
			q3c_fast_get_xy_minmax(face_num, region, region_data, &xmin,
			                       &xmax, &ymin, &ymax);

			xmax = (xmax > Q3C_HALF ? Q3C_HALF : xmax);
			xmin = (xmin < -Q3C_HALF ? -Q3C_HALF : xmin);
			ymax = (ymax > Q3C_HALF ? Q3C_HALF : ymax);
			ymin = (ymin < -Q3C_HALF ? -Q3C_HALF : ymin);
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

			ixmin = (Q3C_HALF + xmin) * n0;
			ixmax = (Q3C_HALF + xmax) * n0;
			iymin = (Q3C_HALF + ymin) * n0;
			iymax = (Q3C_HALF + ymax) * n0;

			ixmax = (ixmax == n0 ? n0 - 1 : ixmax);
			iymax = (iymax == n0 ? n0 - 1 : iymax);

			n1 = nside / n0;

			xistack[1] = (q3c_ipix_t)(ixmin * n1);
			yistack[1] = (q3c_ipix_t)(iymin * n1);
			facestack[1] = face_num;
			nstack[1] = n1;
			nistack = 2;


			face_num = q3c_xy2facenum(2 * points[2], 2 * points[3], face_num0);
			q3c_fast_get_xy_minmax(face_num, region, region_data, &xmin,
			                       &xmax, &ymin, &ymax);

			xmax = (xmax > Q3C_HALF ? Q3C_HALF : xmax);
			xmin = (xmin < -Q3C_HALF ? -Q3C_HALF : xmin);
			ymax = (ymax > Q3C_HALF ? Q3C_HALF : ymax);
			ymin = (ymin < -Q3C_HALF ? -Q3C_HALF : ymin);
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

			ixmin = (Q3C_HALF + xmin) * n0;
			ixmax = (Q3C_HALF + xmax) * n0;
			iymin = (Q3C_HALF + ymin) * n0;
			iymax = (Q3C_HALF + ymax) * n0;

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

#ifdef Q3C_DEBUG
	fprintf(stderr, "MULTI_FLAG: %d\n", multi_flag);
#endif


	/* Now I produce the final pixel value by converting x and y values to bitfields
	    and combining them by interleaving, using the predefined arrays xbits and ybits
	 */

	for(i = 0; i < nistack; i++)
	{
		face_num = facestack[i];
		xi = xistack[i];
		yi = yistack[i];
		n1 = nstack[i];

		ipix0 = q3c_xiyi2ipix(nside, xbits, ybits, face_num, xi, yi);

		*(ipix_cur++) = ipix0;
		*(ipix_cur++) = ipix0 + n1 * n1 - 1;
		/* IMPORTANT!! I subtract 1 to make after the query with <=ipix<=
		 */
	}

	for(; i < 4; i++)
	{
		*(ipix_cur++) = 1;
		*(ipix_cur++) = -1;
	}

}


/* Converts integer coordinates on cube face to
 * ipix number by performing bit interleaving
 */
q3c_ipix_t q3c_xiyi2ipix(q3c_ipix_t nside, q3c_ipix_t *xbits,
                         q3c_ipix_t *ybits, char face_num,
                         q3c_ipix_t xi, q3c_ipix_t yi)
{

	return ((q3c_ipix_t)face_num) * nside * nside +
	       xbits[xi % Q3C_I1] + ybits[yi % Q3C_I1] +
	       (xbits[(xi >> Q3C_INTERLEAVED_NBITS) % Q3C_I1] +
	        ybits[(yi >> Q3C_INTERLEAVED_NBITS) % Q3C_I1]) * Q3C_I1 * Q3C_I1;
	/*8byte computation*/
}


/* convert ipix number ra,dec in degrees */
void q3c_ipix2ang(struct q3c_prm *hprm, q3c_ipix_t ipix,
                  q3c_coord_t *ra, q3c_coord_t *dec)
{
	q3c_ipix_t nside = hprm->nside, ipix1, *xbits1=hprm->xbits1,
	           *ybits1 = hprm->ybits1, i2, i3, x0, y0;

	q3c_coord_t x, y, ra0;
	char face_num = ipix / (nside * nside);
	const q3c_ipix_t ii1 = 1 << (Q3C_INTERLEAVED_NBITS / 2);
	ipix1 = ipix % (nside * nside);

	i3 = ipix1 % Q3C_I1;
	i2 = ipix1 / Q3C_I1;
	x0 = xbits1[i3];
	y0 = ybits1[i3];
	i3 = i2 % Q3C_I1;
	i2 = i2 / Q3C_I1;
	x0 += xbits1[i3] * ii1;
	y0 += ybits1[i3] * ii1;
	i3 = i2 % Q3C_I1;
	i2 = i2 / Q3C_I1;
	x0 += xbits1[i3] * ii1 * ii1;
	y0 += ybits1[i3] * ii1 * ii1;
	i3 = i2 % Q3C_I1;
	i2 = i2 / Q3C_I1;
	x0 += xbits1[i3] * ii1 * ii1 * ii1;
	y0 += ybits1[i3] * ii1 * ii1 * ii1;
	/*
	   BIT_PRINT8(ipix);
	   BIT_PRINT8ix(x0);
	   BIT_PRINT8iy(y0);
	 */

	x = (((q3c_coord_t)x0) / nside) * 2 - 1;
	y = (((q3c_coord_t)y0) / nside) * 2 - 1;
	/* Now -1<x<1 and -1<y<1 */

	/*fprintf(stdout,"%Lf %Lf %Lf %Lf\n",(x+1)/2,(y+1)/2,x,y);*/


	if ((face_num >= 1) && (face_num <= 4))
	{
		ra0 = q3c_atan(x);
		*dec = Q3C_RADEG * q3c_atan(y * q3c_cos(ra0));
		ra0 = ra0 * Q3C_RADEG + ((q3c_coord_t)face_num - 1) * 90;
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
			ra0 = Q3C_RADEG * (q3c_atan2(-x, y) + Q3C_PI);
			*dec = Q3C_RADEG * q3c_atan( 1 / q3c_sqrt(x * x + y * y));
			*ra = ra0;
		}
		if (face_num == 5)
		{
			ra0 = Q3C_RADEG * (q3c_atan2(-x, -y) + Q3C_PI);
			*dec = -Q3C_RADEG * q3c_atan(1 / q3c_sqrt(x * x + y * y));
			*ra = ra0;
		}
	}
	/*fprintf(stdout,"XXX %d %.20Lf %.20Lf\n",face_num,*ra,*dec);*/
}


/* Compute the area of a given Q3C pixel for a given ipix and depth
 * depth here goes from 1 to 30 in the case of 8byte ints
 * and means depth == 1 pixel is the the smallest pixel, and depth== 30 pixel
 * is the whole cube face
 */
q3c_coord_t q3c_pixarea(struct q3c_prm *hprm, q3c_ipix_t ipix, int depth)
{
/* The first part of this function's text was taken from q3c_ipix2ang()
 * In the future I should split the ipix2xy and xy2ang codepaths
 * and put them in the separate functions
 */
	q3c_ipix_t nside = hprm->nside, ipix1, *xbits1=hprm->xbits1,
	           *ybits1 = hprm->ybits1, i2, i3, x0, y0, idx,
	           ix1, iy1, ix2, iy2;
	q3c_coord_t x1, y1, x2, y2, result;
/*	char face_num = ipix / (nside * nside);*/
	const q3c_ipix_t ii1 = 1 << (Q3C_INTERLEAVED_NBITS / 2);
	ipix1 = ipix % (nside * nside);

	i3 = ipix1 % Q3C_I1;
	i2 = ipix1 / Q3C_I1;
	x0 = xbits1[i3];
	y0 = ybits1[i3];
	i3 = i2 % Q3C_I1;
	i2 = i2 / Q3C_I1;
	x0 += xbits1[i3] * ii1;
	y0 += ybits1[i3] * ii1;
	i3 = i2 % Q3C_I1;
	i2 = i2 / Q3C_I1;
	x0 += xbits1[i3] * ii1 * ii1;
	y0 += ybits1[i3] * ii1 * ii1;
	i3 = i2 % Q3C_I1;
	i2 = i2 / Q3C_I1;
	x0 += xbits1[i3] * ii1 * ii1 * ii1;
	y0 += ybits1[i3] * ii1 * ii1 * ii1;
	/*
	   BIT_PRINT8(ipix);
	   BIT_PRINT8ix(x0);
	   BIT_PRINT8iy(y0);
	 */

	ix1 = (x0 >> depth) << depth;
	iy1 = (y0 >> depth) << depth;
	idx = ((q3c_ipix_t)1)<<depth;
	ix2 = ix1 + idx;
	iy2 = iy1 + idx;

	x1 = (((q3c_coord_t)ix1) / nside) * 2 - 1;
	y1 = (((q3c_coord_t)iy1) / nside) * 2 - 1;
	x2 = (((q3c_coord_t)ix2) / nside) * 2 - 1;
	y2 = (((q3c_coord_t)iy2) / nside) * 2 - 1;
	/* Now -1<x<1 and -1<y<1 */

	x1 = x1 / sqrt(1 + x1 * x1);
	y1 = y1 / sqrt(1 + y1 * y1);
	x2 = x2 / sqrt(1 + x2 * x2);
	y2 = y2 / sqrt(1 + y2 * y2);

	if (q3c_fabs(x1-x2)>1e-4)
	{
		result = ( q3c_acos(x1 * y2) - q3c_acos(x1 * y1) ) +
		         ( q3c_acos(x2 * y1) - q3c_acos(x2 * y2) );
	}
	else
	{
		result = q3c_asin(  x1*(y1-y2)*(y1+y2)/(y2*sqrt(1-x1*x1*y1*y1)+y1*sqrt(1-x1*x1*y2*y2)))+
		         q3c_asin(  x2*(y2-y1)*(y1+y2)/(y1*sqrt(1-x2*x2*y2*y2)+y2*sqrt(1-x2*x2*y1*y1)));

	}
	result = q3c_fabs(result);
	return result;
}


/* Find to which facenum a given x,y point on a given face_num0 corresponds
 * The input x, y should be >=-1  and <=1
 */
char q3c_xy2facenum(q3c_coord_t x, q3c_coord_t y, char face_num0)
{
	q3c_coord_t ra = 0, dec = 0;
	/* I do the initialization since gcc warns about probable not
	 * initialization of ra and dec
	 */

	/* This code has been cut out from ipix2ang BEGIN */
	if ((face_num0 >= 1) && (face_num0 <= 4))
	{
		ra = q3c_atan(x);
		dec = Q3C_RADEG * q3c_atan(y * q3c_cos(ra));
		ra = ra * Q3C_RADEG + ((q3c_coord_t)face_num0 - 1) * 90;
		if (ra < 0)
		{
			ra += (q3c_coord_t)360;
		}
	}
	else
	{
		if (face_num0 == 0)
		{
			ra = Q3C_RADEG * q3c_atan2(x, -y);
			dec = Q3C_RADEG * q3c_atan(1 / q3c_sqrt(x * x + y * y));
			if (ra < 0)
			{
				ra += (q3c_coord_t)360;
			}
		}
		if (face_num0 == 5)
		{
			ra = Q3C_RADEG * q3c_atan2(x, y);
			dec = -Q3C_RADEG * q3c_atan(1 / q3c_sqrt(x * x + y * y));
			if (ra < 0)
			{
				ra += (q3c_coord_t)360;
			}

		}
	}
	/* This code has been cut out from ipix2ang END */

	return q3c_get_facenum(ra,dec);
}




/* Initialization of the Q3CUBE structure
 * hprm -- Pointer to main Q3C structure
 * nside -- Nside parameter (number of quadtree subdivisions)
 */
void init_q3c1(struct q3c_prm *hprm, q3c_ipix_t nside)
{
	int i, k, m, l;
	const q3c_ipix_t nbits = Q3C_INTERLEAVED_NBITS;
	/* Number of bits used when interleaving bits
	 * so the size of each allocated array will be 2^16
	 */
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

	xbits[0] = 0;
	xbits[1] = 1;
	ybits[0] = 0;
	ybits[1] = 2;
	/*
	   BIT_PRINT8(xbits[0]);
	   BIT_PRINT8(xbits[1]);
	 */
	for(i = 2, m = 1; i < xybits_size; i++)
	{
		k = i / m;
		if (k == 2)
		{
			xbits[i] = xbits[i / 2] * 4;
			ybits[i] = 2 * xbits[i];
			m *= 2;
			/*
			   BIT_PRINT8(xbits[i]);
			   fprintf(stdout,"%lld\n",ybits[i]);
			 */
			continue;
		}
		else
		{
			xbits[i] = xbits[m] + xbits[i % m];
			ybits[i] = 2 * xbits[i];
			/*
			   BIT_PRINT8(xbits[i]);
			   fprintf(stdout,"%lld\n",ybits[i]);
			 */
			continue;
		}
	}
	xbits1[0] = 0;
	xbits1[1] = 1;

	/*
	   fprintf(stdout,"%lld\n",xbits1[0]);
	   fprintf(stdout,"%lld\n",xbits1[1]);
	   BIT_PRINT8(xbits[0]);
	   BIT_PRINT8(xbits[1]);
	 */

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
		/* fprintf(stdout,"%lld\n",xbits1[i]); */
	}

	ybits1[0] = 0; ybits1[1] = 0;
	/*
	   fprintf(stdout,"%lld\n",ybits1[0]);
	   fprintf(stdout,"%lld\n",ybits1[1]);
	 */

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
		/*fprintf(stdout,"%lld\n",ybits1[i]);*/
	}
}


/* Dump the definitions of  main Q3C arrays into a .c file */
void q3c_dump_prm(struct q3c_prm *hprm,char *filename)
{
	FILE *fp = fopen(filename, "w");
	int i, x = 1 << Q3C_INTERLEAVED_NBITS;
	q3c_ipix_t *xbits = hprm->xbits, *ybits = hprm->ybits,
	           *xbits1 = hprm->xbits1, *ybits1 = hprm->ybits1;

	fprintf(fp, "#include \"common.h\"\n");
	fprintf(fp, "\nq3c_ipix_t ____xbits[%d]={", x);
	fprintf(fp, " ");

	for(i = 0; i < x; i++)
	{
		if (i > 0)
		{
			fprintf(fp, ",");
		}
		fprintf(fp, "Q3C_CONST("Q3C_IPIX_FMT ")", xbits[i]);
	}
	fprintf(fp, "};");

	fprintf(fp, "\nq3c_ipix_t ____ybits[%d]={",x);
	fprintf(fp, " ");

	for(i = 0; i < x; i++)
	{
		if (i > 0)
		{
			fprintf(fp, ",");
		}
		fprintf(fp, "Q3C_CONST("Q3C_IPIX_FMT ")", ybits[i]);
	}
	fprintf(fp, "};");

	fprintf(fp, "\nq3c_ipix_t ____xbits1[%d]={", x);
	fprintf(fp, " ");

	for(i = 0; i < x; i++)
	{
		if (i > 0)
		{
			fprintf(fp, ",");
		}
		fprintf(fp, "Q3C_CONST("Q3C_IPIX_FMT ")", xbits1[i]);
	}
	fprintf(fp, "};");

	fprintf(fp, "\nq3c_ipix_t ____ybits1[%d]={",x);
	fprintf(fp, " ");

	for(i = 0; i < x; i++)
	{
		if (i > 0)
		{
			fprintf(fp, ",");
		}
		fprintf(fp, "Q3C_CONST("Q3C_IPIX_FMT ")", ybits1[i]);
	}
	fprintf(fp, "};\n");

	fprintf(fp, "struct q3c_prm hprm={"
	        Q3C_IPIX_FMT ",____xbits,____ybits,____xbits1,____ybits1};\n", hprm->nside);
	fclose(fp);
}


/* That function computes the coefficients of the 2nd order poly
 * describint the ellipse
 * (axx*x^2+ayy*y^2+2*axy*(x*y)+ax*x+ay*y+a=0)
 * produced on the cube face by the cone search
 */
void q3c_get_poly_coefs(char face_num, q3c_coord_t ra0, q3c_coord_t dec0,
                        q3c_coord_t rad, q3c_coord_t *axx, q3c_coord_t *ayy,
                        q3c_coord_t *axy, q3c_coord_t *ax, q3c_coord_t *ay,
                        q3c_coord_t *a)
{
	q3c_coord_t ra1, dec1, sr, cr, cd, sd, crad, p = 1;

	dec1 = dec0 * Q3C_DEGRA;
	sd = q3c_sin(dec1);
	cd = q3c_cos(dec1);
	crad = q3c_cos(Q3C_DEGRA * rad);

	if ((face_num >= 1) && (face_num <= 4))
	{
		ra1 = (ra0 - (face_num  - 1 ) * 90) * Q3C_DEGRA;
		sr = q3c_sin(ra1);
		cr = q3c_cos(ra1);
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

		ra1 = ra0 * Q3C_DEGRA;
		sr = q3c_sin(ra1);
		cr = q3c_cos(ra1);
		*axx = (crad * crad - sr * sr * cd * cd);
		*ayy = (crad * crad - cr * cr * cd * cd);
		*axy = (2 * p * sr * cr * cd * cd);
		*ax = -(2 * p * sr * sd * cd);
		*ay = (2 * cr * sd * cd);
		*a = crad * crad  - sd * sd;
	}

	/* By this step I convert the coefficient of the polynom to the
	 * coordinate system on the cube face where x[-0.5,0.5] y[-0.5,0.5]
	 * I should incorporate this formulae directly with the formulae
	 * from the top
	 */

	(*axx) *= 4;
	(*ayy) *= 4;
	(*axy) *= 4;
	(*ax) *= 2;
	(*ay) *= 2;
}


/* That function determines the minimal, maximal x and y of the ellipse with
 * the given coefficients (axx,ayy,axy...). All the computations are done on
 * the cube face.
 */
void q3c_get_xy_minmax(q3c_coord_t axx, q3c_coord_t ayy, q3c_coord_t axy,
                       q3c_coord_t ax, q3c_coord_t ay, q3c_coord_t a,
                       q3c_coord_t *xmin, q3c_coord_t *xmax,
                       q3c_coord_t *ymin, q3c_coord_t *ymax,
                       char *full_flag)
{
	q3c_coord_t tmp0, tmp1, tmp2;

	tmp0 = axy * ax - 2 * axx * ay;
	tmp1 = 2 * q3c_sqrt(axx * (axx * ay * ay - axy * ax * ay -
	                           4 * axx * ayy * a + axy * axy * a + ax * ax * ayy));
	tmp2 = 4 * axx * ayy - axy * axy;

	/*
	 * If the discriminant of the curve is smaller than a given threshold, it
	 * means that the curve is (or is close to) a parabola or a hyperbola
	 * In that case I include the whole face, because
	 * I don't know how to compute the intersection of the cube
	 * and the hyperbola
	 */
	if (tmp2< Q3C_MINDISCR)
	{
		*xmax = 2*Q3C_HALF;
		*ymax = 2*Q3C_HALF;
		*xmin = -2*Q3C_HALF;
		*ymin = -2*Q3C_HALF;
		*full_flag = 1;
		return;
	}

	*ymin = (tmp0 - tmp1) / tmp2;
	*ymax = (tmp0 + tmp1) / tmp2;
	tmp0 = axy * ay - 2 * ayy * ax;
	tmp1 = 2 * q3c_sqrt(ayy * (ayy * ax * ax - axy * ax * ay -
	                           4 * axx * ayy * a + axy * axy * a + ay * ay * axx));
	*xmin = (tmp0 - tmp1) / tmp2;
	*xmax = (tmp0 + tmp1) / tmp2;
	//fprintf(stdout,"xmin=%.10Lf xmax=%.10Lf ymin=%.10Lf ymax=%.10Lf\n", *xmin, *xmax, *ymin, *ymax);

}

/* Check if the region is too big for Q3C to handle properly
 * The limit here is related to the number of faces the region is
 * allowed to intersect. Q3C can't handle more than 3 faces.
 * So for such big regions, we just switch to scan of the whole table
 */

char q3c_too_big_check(q3c_region region, void * region_data)
{
	switch (region)
	{
	case Q3C_CIRCLE:
	{
		q3c_circle_region circle = *(q3c_circle_region *)region_data;
		if (circle.rad>Q3C_MAXRAD)
		{
			return 1;
		}
		else
		{
			return 0;
		}
		break;
	}
	case Q3C_ELLIPSE:
	{
		q3c_ellipse_region ellipse = *(q3c_ellipse_region *)region_data;
		if (ellipse.rad>Q3C_MAXRAD)
		{
			return 1;
		}
		else
		{
			return 0;
		}
		break;
	}
	case Q3C_POLYGON:
	{
	}
	}
	/* should not happen */
	return 0; /* DUMMY */


}

/* Get the xmin,ymin,xmax,ymax box on a given face for a given region */
void q3c_fast_get_xy_minmax(char face_num, q3c_region region,
                            void *region_data,
                            q3c_coord_t *xmin, q3c_coord_t *xmax,
                            q3c_coord_t *ymin, q3c_coord_t *ymax)
{

	switch (region)
	{
	case Q3C_CIRCLE:
	{
		q3c_circle_region circle = *(q3c_circle_region *)region_data;
		q3c_fast_get_circle_xy_minmax(face_num, circle.ra, circle.dec, circle.rad, xmin, xmax, ymin, ymax);
		break;
	}
	case Q3C_ELLIPSE:
	{
		q3c_ellipse_region ellipse = *(q3c_ellipse_region *)region_data;
		q3c_fast_get_ellipse_xy_minmax(face_num, ellipse.ra, ellipse.dec, ellipse.rad, ellipse.e, ellipse.PA, xmin, xmax, ymin, ymax);
		break;
	}
	case Q3C_POLYGON:
	{
	}
	}
}


/* That function gets the min, max x and y of the ellipse with the given
 * coefficients (axx,ayy,axy...). All the computations are done on the cube face.
 * That function take as arguments only the ra, dec of the center of cone search
 * and radius.
 */
void q3c_fast_get_circle_xy_minmax(char face_num, q3c_coord_t ra0, q3c_coord_t dec0,
                                   q3c_coord_t rad, q3c_coord_t *xmin,
                                   q3c_coord_t *xmax, q3c_coord_t *ymin,
                                   q3c_coord_t *ymax)
{
	q3c_coord_t tmp0, tmp1, tmp2, ra1, dec1, sr, cr,sd, cd, srad, crad, crad2,
	            cd2, scd;

	dec1 = dec0 * Q3C_DEGRA;
	q3c_sincos(dec1, sd, cd);
	cd2 = cd * cd;
	q3c_sincos(Q3C_DEGRA * rad, srad, crad);
	crad2 = crad * crad;
	/* tmp1, tmp2  variables in this function will
	 * be related to the coefficients of quadratic equations
	 * for xmin,xmax,ymin,ymax
	 */

	if ((face_num >= 1) && (face_num <= 4))
	{
		ra1 = (ra0 - (face_num - 1 ) * 90) * Q3C_DEGRA;
		q3c_sincos(ra1, sr, cr);

		tmp2 = 2 * ( cd2 * cr * cr - srad * srad );
		/* tmp2 is the discriminant of the curve on the face
		 * zero or negative values mean parabolas or hyperbolas
		 * in that case we better take whole face
		 */
		if (tmp2 < Q3C_MINDISCR)
		{
			*xmin = -Q3C_HALF;
			*ymin = -Q3C_HALF;
			*xmax = Q3C_HALF;
			*ymax = Q3C_HALF;
			return;
		}
		tmp0 = sr * cr *cd2;
		tmp1 = srad * q3c_sqrt(cd2 - srad * srad);
		*xmin = (tmp0 - tmp1) / tmp2;
		*xmax = (tmp0 + tmp1) / tmp2;
		tmp0 = cr * cd * sd;
		tmp1 = srad * q3c_sqrt(crad2 - cd2 * sr * sr);
		*ymin = (tmp0 - tmp1) / tmp2;
		*ymax = (tmp0 + tmp1) / tmp2;
	}
	else
	{
		ra1 = ra0 * Q3C_DEGRA;
		q3c_sincos(ra1, sr, cr);
		scd = sd * cd;
		tmp0 = scd * sr;
		tmp1 = srad * q3c_sqrt(crad2 - cr * cr * cd2);
		tmp2 = 2 * (crad2 - cd2);
		if (tmp2 < Q3C_MINDISCR)
		{
			/* tmp2 is the discriminant of the curve on the face
			 * zero or negative values mean parabolas or hyperbolas
			 * in that case we better take whole face
			 */
			*xmin = -Q3C_HALF;
			*ymin = -Q3C_HALF;
			*xmax = Q3C_HALF;
			*ymax = Q3C_HALF;
			return;
		}

		if (face_num == 5) tmp0 = -tmp0;

		*xmin = (tmp0 - tmp1) / tmp2;
		*xmax = (tmp0 + tmp1) / tmp2;
		tmp0 = -scd * cr;
		tmp1 = srad * q3c_sqrt(crad2 - sr * sr * cd2);
		*ymin = (tmp0 - tmp1) / tmp2;
		*ymax = (tmp0 + tmp1) / tmp2;
	}
}

/* Get the xmin,ymin,xmax,ymax on equatorial cube faces for
 * elliptic queries
 */
void q3c_fast_get_equatorial_ellipse_xy_minmax(q3c_coord_t alpha,
                                               q3c_coord_t delta,
                                               q3c_coord_t d, q3c_coord_t e,
                                               q3c_coord_t PA,
                                               q3c_coord_t *ymin,
                                               q3c_coord_t *ymax,
                                               q3c_coord_t *zmin,
                                               q3c_coord_t *zmax)
{
	/* Thank you, Maple! */
	q3c_coord_t t1 = q3c_sin(alpha);
	q3c_coord_t t2 = q3c_cos(alpha);
	q3c_coord_t t21 = q3c_sin(delta);
	q3c_coord_t t4 = q3c_cos(delta);
	q3c_coord_t t24 = q3c_sin(PA);
	q3c_coord_t t13 = q3c_cos(PA);
	q3c_coord_t t51 = q3c_sin(d);
	q3c_coord_t t8 = q3c_cos(d);

	q3c_coord_t t3 = t1*t2;
	q3c_coord_t t5 = t4*t4;
	q3c_coord_t t7 = 2.0*t3*t5;
	q3c_coord_t t9 = t8*t8;
	q3c_coord_t t12 = t1*t5;
	q3c_coord_t t14 = t13*t13;
	q3c_coord_t t15 = t2*t14;
	q3c_coord_t t22 = t21*t13;
	q3c_coord_t t23 = t2*t2;
	q3c_coord_t t34 = t5*t9;
	q3c_coord_t t39 = t24*t9;
	q3c_coord_t t47 = -t7-2.0*t3*t9+2.0*t12*t15-2.0*t12*t15*t9+4.0*t22*t23*t24-4.0*t15*t1+4.0*t1*t9*t15+2.0*t3+2.0*t3*t34-2.0*t22*t24+2.0*t22*t39-4.0*t21*t9*t13*t23*t24;
	q3c_coord_t t48 = e*e;
	q3c_coord_t t52 = t51*t51;
	q3c_coord_t t54 = t5*t14*t9;
	q3c_coord_t t61 = t23*t5;
	q3c_coord_t t62 = 2.0*t61;
	q3c_coord_t t63 = t23*t14;
	q3c_coord_t t67 = t23*t9;
	q3c_coord_t t69 = t61*t9;
	q3c_coord_t t71 = t14*t9;
	q3c_coord_t t73 = t1*t21;
	q3c_coord_t t77 = t73*t13*t2*t24*t9;
	q3c_coord_t t79 = t71*t23;
	q3c_coord_t t85 = t63*t34;
	q3c_coord_t t89 = -t62-4.0*t63+2.0*t63*t5-2.0*t67+2.0*t69-2.0*t71+4.0*t77+4.0*t79-4.0*t73*t13*t2*t24-2.0*t85+2.0*t14+2.0*t23;
	q3c_coord_t t92 = t89*t48-2.0+2.0*t9+t62;
	q3c_coord_t t93 = t1*t13;
	q3c_coord_t t96 = t21*t2;

	q3c_coord_t tmpy0 = t47*t48+t7;
	q3c_coord_t tmpy1 = -4.0*t52*(t9-1.0+t54+t5-t34)*t48+4.0*t52*(-1.0+t9+t5);
	q3c_coord_t tmpy2 = t92;

	q3c_coord_t tmpz0 = -2.0*(-t93*t24+t93*t39+t96+t96*t71-t96*t14-t96*t9)*t4*t48+2.0*t96*t4;
	q3c_coord_t tmpz1 = -4.0*t52*(t61-2.0*t79-t54+t67-2.0*t77-t5-t69+t71+t85+t34)*t48+4.0*t52*(t9+t61-t5);
	q3c_coord_t tmpz2 = t92;

	tmpy1 = q3c_sqrt(tmpy1);
	tmpy2 = (2 * tmpy2);
	tmpz1 = q3c_sqrt(tmpz1);
	tmpz2 = (2 * tmpz2);
	if (tmpy2 < Q3C_MINDISCR)
	{
		*ymin = -Q3C_HALF;
		*ymax = -Q3C_HALF;
		*zmin = Q3C_HALF;
		*zmax = Q3C_HALF;
		return;
	}

	*ymin = (tmpy0 - tmpy1) / tmpy2;
	*ymax = (tmpy0 + tmpy1) / tmpy2;
	*zmin = (tmpz0 - tmpz1) / tmpz2;
	*zmax = (tmpz0 + tmpz1) / tmpz2;

}


/* Get the xmin,ymin,xmax,ymax on polar cube faces for
 * elliptic queries
 * !!IMPORTANT!! for south pole the ycoordinates (1st) should be inverted
 */
void q3c_fast_get_polar_ellipse_xy_minmax(q3c_coord_t alpha, q3c_coord_t delta,
                                          q3c_coord_t d, q3c_coord_t e,
                                          q3c_coord_t PA, q3c_coord_t *ymin,
                                          q3c_coord_t *ymax,
                                          q3c_coord_t *zmin,
                                          q3c_coord_t *zmax)
{
/* Thank you, Maple! */
	q3c_coord_t t1 = q3c_sin(alpha);
	q3c_coord_t t14 = q3c_cos(alpha);
	q3c_coord_t t2 = q3c_sin(delta);
	q3c_coord_t t19 = q3c_cos(delta);
	q3c_coord_t t12 = q3c_sin(PA);
	q3c_coord_t t7 = q3c_cos(PA);
	q3c_coord_t t25 = q3c_sin(d);
	q3c_coord_t t4 = q3c_cos(d);

	q3c_coord_t t3 = t1*t2;
	q3c_coord_t t5 = t4*t4;
	q3c_coord_t t6 = t3*t5;
	q3c_coord_t t8 = t7*t7;
	q3c_coord_t t10 = t8*t5;
	q3c_coord_t t13 = t7*t12;
	q3c_coord_t t15 = t13*t14;
	q3c_coord_t t21 = e*e;
	q3c_coord_t t26 = t25*t25;
	q3c_coord_t t28 = 2.0*t6*t15;
	q3c_coord_t t29 = t19*t19;
	q3c_coord_t t30 = t14*t14;
	q3c_coord_t t31 = t29*t30;
	q3c_coord_t t32 = t31*t5;
	q3c_coord_t t34 = 2.0*t10*t30;
	q3c_coord_t t35 = t30*t5;
	q3c_coord_t t36 = t31*t10;
	q3c_coord_t t46 = t29*(1.0-t5-t8+t10)*t21+t5-t29;
	q3c_coord_t t47 = t7*t1;
	q3c_coord_t t51 = t14*t2;

	q3c_coord_t tmpy0 = 2.0*(t6+t3*t8-t3*t10-t15+t13*t14*t5-t3)*t19*t21+2.0*t3*t19;
	q3c_coord_t tmpy1 = 4.0*t26*(-t5-t28-t32-t34+t35+t36+t31+t10)*t21-4.0*t26*(-t5+t31);
	q3c_coord_t tmpy2 = 2.0*t46;

	q3c_coord_t tmpz0 = 2.0*(-t47*t12+t47*t12*t5+t51*t10-t51*t5-t51*t8+t51)*t19*t21-2.0*t51*t19;
	q3c_coord_t tmpz1 = -4.0*t26*(-t28-t29*t8*t5-t29-t32-t34+t35+t36+t31+t29*t5+t10)*t21+4.0*t26*(t5-t29+t31);
	q3c_coord_t tmpz2 = 2.0*t46;

	tmpy1 = q3c_sqrt(tmpy1);
	tmpy2 = (2 * tmpy2);
	tmpz1 = q3c_sqrt(tmpz1);
	tmpz2 = (2 * tmpz2);

	if (tmpy2 < Q3C_MINDISCR)
	{
		*ymin = -Q3C_HALF;
		*ymax = -Q3C_HALF;
		*zmin = Q3C_HALF;
		*zmax = Q3C_HALF;
		return;
	}

	*ymin = (tmpy0 - tmpy1) / tmpy2;
	*ymax = (tmpy0 + tmpy1) / tmpy2;
	*zmin = (tmpz0 - tmpz1) / tmpz2;
	*zmax = (tmpz0 + tmpz1) / tmpz2;
}




/* get the xmin,xmax,ymin,ymax and polynomial coefficients for the ellipse query
 * and equatorial cube faces
 */
void q3c_fast_get_equatorial_ellipse_xy_minmax_and_poly_coefs(q3c_coord_t alpha,
                                                              q3c_coord_t delta,
                                                              q3c_coord_t d, q3c_coord_t e,
                                                              q3c_coord_t PA,
                                                              q3c_coord_t *ymin,
                                                              q3c_coord_t *ymax,
                                                              q3c_coord_t *zmin,
                                                              q3c_coord_t *zmax,
                                                              q3c_coord_t *ayy,
                                                              q3c_coord_t *azz,
                                                              q3c_coord_t *ayz,
                                                              q3c_coord_t *ay,
                                                              q3c_coord_t *az,
                                                              q3c_coord_t *a)
{
	/* Thank you, Maple! */
	q3c_coord_t t1 = q3c_sin(delta);
	q3c_coord_t t2 = q3c_cos(d);
	q3c_coord_t t3 = t2*t2;
	q3c_coord_t t4 = t1*t3;
	q3c_coord_t t5 = q3c_cos(PA);
	q3c_coord_t t6 = q3c_cos(alpha);
	q3c_coord_t t7 = t6*t6;
	q3c_coord_t t9 = q3c_sin(PA);
	q3c_coord_t t13 = t5*t5;
	q3c_coord_t t14 = t6*t13;
	q3c_coord_t t15 = q3c_sin(alpha);
	q3c_coord_t t18 = t15*t3;
	q3c_coord_t t19 = t18*t14;
	q3c_coord_t t21 = q3c_cos(delta);
	q3c_coord_t t22 = t21*t21;
	q3c_coord_t t23 = t15*t22;
	q3c_coord_t t29 = t15*t6;
	q3c_coord_t t31 = t22*t3;
	q3c_coord_t t34 = t1*t5;
	q3c_coord_t t37 = t9*t3;
	q3c_coord_t t43 = 2.0*t29*t22;
	q3c_coord_t t44 = t7*t9;
	q3c_coord_t t47 = -4.0*t4*t5*t7*t9-4.0*t14*t15+4.0*t19+2.0*t14*t23-2.0*t14*t23*t3+2.0*t29+2.0*t29*t31-2.0*t34*t9+2.0*t34*t37-2.0*t29*t3-t43+4.0*t34*t44;
	q3c_coord_t t48 = e*e;
	q3c_coord_t t51 = q3c_sin(d);
	q3c_coord_t t52 = t51*t51;
	q3c_coord_t t53 = t22*t13;
	q3c_coord_t t54 = t53*t3;
	q3c_coord_t t61 = t15*t1;
	q3c_coord_t t62 = t61*t3;
	q3c_coord_t t63 = t5*t6;
	q3c_coord_t t64 = t63*t9;
	q3c_coord_t t65 = t62*t64;
	q3c_coord_t t67 = t13*t3;
	q3c_coord_t t70 = t7*t3;
	q3c_coord_t t71 = t53*t70;
	q3c_coord_t t74 = t22*t7;
	q3c_coord_t t75 = 2.0*t74;
	q3c_coord_t t78 = t74*t3;
	q3c_coord_t t80 = t67*t7;
	q3c_coord_t t87 = 4.0*t65-2.0*t67+2.0*t13-2.0*t71-2.0*t70-t75-4.0*t7*t13+2.0*t78+4.0*t80+2.0*t74*t13+2.0*t7-4.0*t61*t64;
	q3c_coord_t t90 = t87*t48-2.0+t75+2.0*t3;
	q3c_coord_t t91 = t1*t6;
	q3c_coord_t t94 = t15*t5;
	q3c_coord_t t101 = t91*t21;
	q3c_coord_t t111 = t9*t9;
	q3c_coord_t t112 = t3*t111;
	q3c_coord_t t113 = t15*t15;
	q3c_coord_t t117 = t52*t48;
	q3c_coord_t t124 = 2.0*t62*t63*t9*t48;
	q3c_coord_t t125 = t1*t1;
	q3c_coord_t t126 = t7*t125;
	q3c_coord_t t132 = t4*t5;
	q3c_coord_t t138 = t6*t3;
	q3c_coord_t t143 = t22*t52;
	q3c_coord_t t145 = t111*t6;
	q3c_coord_t t159 = t21*t9*t3;
	q3c_coord_t t160 = t5*t48;
	q3c_coord_t t163 = t67*t48;
	q3c_coord_t t165 = t52*t21;
	q3c_coord_t t167 = t165*t48;
	q3c_coord_t t170 = t21*t13*t3;
	q3c_coord_t t173 = t21*t111*t3;
	q3c_coord_t t186 = t113*t22;
	q3c_coord_t t190 = t113*t125;
	q3c_coord_t t197 = t125*t52;
	q3c_coord_t tmpy0 = t47*t48+t43;
	q3c_coord_t tmpy1 = -4.0*t52*(-1.0+t54+t3+t22-t31)*t48+4.0*t52*(-1.0+t22+t3);
	q3c_coord_t tmpy2 = t90;
	q3c_coord_t tmpz0 = -2.0*(-t91*t13-t91*t3+t94*t37+t91+t91*t67-t94*t9)*t21*t48+2.0*t101;
	q3c_coord_t tmpz1 = 4.0*t52*(-t67-t74-t70+t54+2.0*t80+t78-t71+t22-t31+2.0*t65)*t48+4.0*t52*(t3+t74-t22);
	q3c_coord_t tmpz2 = t90;
	*a = -t112*t113-t67*t113+t74*t52-t74*t117+t112*t113*t48+t124+t67*t126*t48-t67*t126-t112*t126;
	*ay = -2.0*t132*t44*t48+2.0*t132*t113*t9*t48+2.0*t138*t13*t15*t125*t48+2.0*t29*t143+2.0*t18*t145+2.0*t19-2.0*t18*t145*t48-2.0*t138*t13*t15*t125-2.0*t29*t143*t48-2.0*t138*t111*t15*t125;
	*az = -2.0*t159*t160*t15-2.0*t101*t163+2.0*t91*t165-2.0*t91*t167+2.0*t91*t170+2.0*t91*t173;
	*ayz = 2.0*t159*t160*t6-2.0*t61*t21*t163+2.0*t61*t165+2.0*t61*t173-2.0*t61*t167+2.0*t61*t170;
	*ayy = -t112*t7-t80+t186*t52+t112*t7*t48-t67*t190-t112*t190-t186*t117+t67*t190*t48-t124;
	*azz = t197+t53*t3*t48-t54-t197*t48-t22*t111*t3;
	tmpy1 = q3c_sqrt(tmpy1);
	tmpy2 = (2 * tmpy2);
	tmpz1 = q3c_sqrt(tmpz1);
	tmpz2 = (2 * tmpz2);

	*ymin = (tmpy0 - tmpy1) / tmpy2;
	*ymax = (tmpy0 + tmpy1) / tmpy2;
	*zmin = (tmpz0 - tmpz1) / tmpz2;
	*zmax = (tmpz0 + tmpz1) / tmpz2;

	/* reduce the values to the cube with edge length of 1 (instead of 2) */
	*ayy*=-4;
	*azz*=-4;
	*ayz*=-4;
	*ay*=-2;
	*az*=-2;
	*a*=-1;
}

/* get the xmin,xmax,ymin,ymax and polynomial coefficients for the ellipse query
 * and polar cube faces
 */
void q3c_fast_get_polar_ellipse_xy_minmax_and_poly_coefs(q3c_coord_t alpha,
                                                         q3c_coord_t delta,
                                                         q3c_coord_t d, q3c_coord_t e,
                                                         q3c_coord_t PA,
                                                         q3c_coord_t *ymin,
                                                         q3c_coord_t *ymax,
                                                         q3c_coord_t *zmin,
                                                         q3c_coord_t *zmax,
                                                         q3c_coord_t *ayy,
                                                         q3c_coord_t *azz,
                                                         q3c_coord_t *ayz,
                                                         q3c_coord_t *ay,
                                                         q3c_coord_t *az,
                                                         q3c_coord_t *a)
{
	/* Thank you, Maple! */
	q3c_coord_t t1 = q3c_sin(alpha);
	q3c_coord_t t2 = q3c_sin(delta);
	q3c_coord_t t3 = t1*t2;
	q3c_coord_t t4 = q3c_cos(d);
	q3c_coord_t t5 = t4*t4;
	q3c_coord_t t6 = t3*t5;
	q3c_coord_t t7 = q3c_cos(PA);
	q3c_coord_t t8 = t7*t7;
	q3c_coord_t t9 = t8*t5;
	q3c_coord_t t11 = q3c_sin(PA);
	q3c_coord_t t12 = t7*t11;
	q3c_coord_t t13 = q3c_cos(alpha);
	q3c_coord_t t14 = t12*t13;
	q3c_coord_t t15 = t13*t5;
	q3c_coord_t t19 = q3c_cos(delta);
	q3c_coord_t t21 = e*e;
	q3c_coord_t t23 = t3*t19;
	q3c_coord_t t25 = q3c_sin(d);
	q3c_coord_t t26 = t25*t25;
	q3c_coord_t t28 = 2.0*t6*t14;
	q3c_coord_t t29 = t19*t19;
	q3c_coord_t t30 = t13*t13;
	q3c_coord_t t31 = t29*t30;
	q3c_coord_t t32 = t31*t5;
	q3c_coord_t t33 = t29*t8;
	q3c_coord_t t34 = t30*t5;
	q3c_coord_t t35 = t33*t34;
	q3c_coord_t t36 = t9*t30;
	q3c_coord_t t37 = 2.0*t36;
	q3c_coord_t t47 = t29*(1.0-t5-t8+t9)*t21-t29+t5;
	q3c_coord_t t48 = t7*t1;
	q3c_coord_t t52 = t13*t2;
	q3c_coord_t t59 = t52*t19;
	q3c_coord_t t61 = t33*t5;
	q3c_coord_t t71 = t11*t11;
	q3c_coord_t t74 = t2*t2;
	q3c_coord_t t75 = t74*t26;
	q3c_coord_t t79 = t19*t71*t5;
	q3c_coord_t t82 = t19*t8*t5;
	q3c_coord_t t84 = t26*t19;
	q3c_coord_t t85 = t84*t21;
	q3c_coord_t t87 = t9*t21;
	q3c_coord_t t90 = t19*t11*t5;
	q3c_coord_t t91 = t7*t21;
	q3c_coord_t t112 = t2*t5*t7;
	q3c_coord_t t113 = t1*t1;
	q3c_coord_t t120 = t13*t1;
	q3c_coord_t t121 = t29*t26;
	q3c_coord_t t123 = t1*t5;
	q3c_coord_t t126 = t71*t13;
	q3c_coord_t t136 = t113*t29;
	q3c_coord_t t138 = t5*t71;
	q3c_coord_t t144 = 2.0*t6*t7*t13*t11*t21;
	q3c_coord_t t147 = t113*t74;
	q3c_coord_t t150 = t26*t21;
	q3c_coord_t t157 = t30*t74;

	q3c_coord_t tmpy0 = 2.0*(-t3+t6-t3*t9-t14+t12*t15+t3*t8)*t19*t21+2.0*t23;
	q3c_coord_t tmpy1 = 4.0*t26*(-t5-t28+t31-t32+t35-t37+t34+t9)*t21-4.0*t26*(t31-t5);
	q3c_coord_t tmpy2 = 2.0*t47;
	q3c_coord_t tmpz0 = 2.0*(-t48*t11+t48*t11*t5+t52-t52*t5+t52*t9-t52*t8)*t19*t21-2.0*t59;
	q3c_coord_t tmpz1 = -4.0*t26*(-t29-t61-t28+t31-t32+t35-t37+t34+t29*t5+t9)*t21+4.0*t26*(-t29+t31+t5);
	q3c_coord_t tmpz2 = 2.0*t47;
	*a = t33*t5*t21-t29*t71*t5-t61-t75*t21+t75;
	*ay = 2.0*t3*t79+2.0*t3*t82-2.0*t3*t85-2.0*t23*t87+2.0*t90*t91*t13+2.0*t3*t84;
	*az = -2.0*t52*t84+2.0*t52*t85-2.0*t52*t82-2.0*t52*t79+2.0*t90*t91*t1+2.0*t59*t87;
	*ayz = 2.0*t15*t8*t1*t74-2.0*t15*t8*t1*t74*t21-2.0*t112*t113*t11*t21+2.0*t112*t30*t11*t21-2.0*t120*t121-2.0*t123*t8*t13-2.0*t123*t126+2.0*t15*t71*t1*t74+2.0*t120*t121*t21+2.0*t123*t126*t21;
	*ayy = -t36+t136*t26-t138*t30-t144+t138*t30*t21-t9*t147-t138*t147-t136*t150+t9*t147*t21;
	*azz = t138*t113*t21-t9*t157-t138*t157-t31*t150+t9*t157*t21+t144-t138*t113-t9*t113+t31*t26;

	tmpy1 = q3c_sqrt(tmpy1);
	tmpy2 = (2 * tmpy2);
	tmpz1 = q3c_sqrt(tmpz1);
	tmpz2 = (2 * tmpz2);

	*ymin = (tmpy0 - tmpy1) / tmpy2;
	*ymax = (tmpy0 + tmpy1) / tmpy2;
	*zmin = (tmpz0 - tmpz1) / tmpz2;
	*zmax = (tmpz0 + tmpz1) / tmpz2;
	/* reduce the values to the cube with edge length of 1 (instead of 2) */
	*ayy*=-4;
	*azz*=-4;
	*ayz*=-4;
	*ay*=-2;
	*az*=-2;
	*a*=-1;
}


/* Get ellipse x,y min,max and poly coeffs for a given ellipse query */
void q3c_fast_get_ellipse_xy_minmax_and_poly_coefs(char face_num,
                                                   q3c_coord_t ra0,
                                                   q3c_coord_t dec0,
                                                   q3c_coord_t d0,
                                                   q3c_coord_t e,
                                                   q3c_coord_t PA0,
                                                   q3c_coord_t *ymin,
                                                   q3c_coord_t *ymax,
                                                   q3c_coord_t *zmin,
                                                   q3c_coord_t *zmax,
                                                   q3c_coord_t *ayy,
                                                   q3c_coord_t *azz,
                                                   q3c_coord_t *ayz,
                                                   q3c_coord_t *ay,
                                                   q3c_coord_t *az,
                                                   q3c_coord_t *a)
{
	q3c_coord_t ra1, dec1 = dec0 * Q3C_DEGRA, d1 = d0 * Q3C_DEGRA,
	            PA1 = PA0 * Q3C_DEGRA, tmpx;

	if ((face_num > 0) && (face_num < 5))
	{
		ra1 = (ra0 - (face_num - 1) * 90) * Q3C_DEGRA;
		q3c_fast_get_equatorial_ellipse_xy_minmax_and_poly_coefs(
			ra1, dec1, d1, e, PA1, ymin, ymax, zmin, zmax,
			ayy, azz, ayz, ay, az, a);
	}
	else
	{
		ra1 = ra0 * Q3C_DEGRA;
		q3c_fast_get_polar_ellipse_xy_minmax_and_poly_coefs(
			ra1, dec1, d1, e, PA1, ymin, ymax, zmin, zmax,
			ayy, azz, ayz, ay, az, a);

		if (face_num == 5)
		{
			tmpx = *ymin;
			*ymin = -(*ymax);
			*ymax = -tmpx;
			*ay=-*ay;
			*ayz=-*ayz;

		}
	}
}



void q3c_fast_get_ellipse_xy_minmax(char face_num, q3c_coord_t ra0,
                                    q3c_coord_t dec0, q3c_coord_t rad0,
                                    q3c_coord_t e, q3c_coord_t PA0,
                                    q3c_coord_t *xmin, q3c_coord_t *xmax,
                                    q3c_coord_t *ymin, q3c_coord_t *ymax)
{
	q3c_coord_t ra1, dec1 = dec0 * Q3C_DEGRA, rad1 = rad0 * Q3C_DEGRA,
	            PA1 = PA0 * Q3C_DEGRA, tmpx;

	if ((face_num > 0) && (face_num < 5))
	{
		ra1 = (ra0 - (face_num - 1) * 90) * Q3C_DEGRA;
		q3c_fast_get_equatorial_ellipse_xy_minmax(ra1, dec1, rad1, e, PA1,
		                                          xmin, xmax, ymin, ymax);
	}
	else
	{
		ra1 = ra0 * Q3C_DEGRA;
		q3c_fast_get_polar_ellipse_xy_minmax(ra1, dec1, rad1, e, PA1,
		                                     xmin, xmax, ymin, ymax);
		if (face_num == 5)
		{
			/* We have to invert one axis in the case of south pole face */
			tmpx = *xmin;
			*xmin = -(*xmax);
			*xmax = -tmpx;
		}
	}
}


/* Function checking whether the square with center xc_cur, yc_cur and the
 * size cur_size on the cube face lie inside, or intersects etc. with the ellipse
 * specified by the coefficients (axx, axy, ayy, ax, ay, a)
 */
static char q3c_circle_cover_check(q3c_coord_t xc_cur, q3c_coord_t yc_cur,
                                   q3c_coord_t cur_size, q3c_coord_t xmin,
                                   q3c_coord_t xmax, q3c_coord_t ymin,
                                   q3c_coord_t ymax, q3c_coord_t axx,
                                   q3c_coord_t axy, q3c_coord_t ayy,
                                   q3c_coord_t ax, q3c_coord_t ay,
                                   q3c_coord_t a)
{
	q3c_coord_t xl_cur, xr_cur, yb_cur, yt_cur;
	char status = 0, inside;

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

	#define EVAL_POLY(x,y) ( x * (axx * x + axy * y + ax) + y * (ayy * y + ay) + a )

	/* the idea of the code is following:
	 * we go through the list of vertices till we encounter the one
	 * which is inside the ellipse
	 * If none of the vertices is inside the ellipse we double check the
	 * edge crossings
	 * If all the vertices are inside we return "covered"
	 */

	/* Bottom left vertex */
	inside  = EVAL_POLY(xl_cur, yb_cur) < 0;
	status += inside;

	/* Bottom right vertex */
	inside = EVAL_POLY(xr_cur, yb_cur) < 0;
	/* we use XOR to check if the current vertex has different status
	 * than the previous ones */
	if (inside ^ (status > 0))
	{
		return Q3C_PARTIAL;
	}
	status += inside;

	/* Top right vertex */
	inside = EVAL_POLY(xr_cur, yt_cur) < 0;
	if (inside ^ (status > 0))
	{
		return Q3C_PARTIAL;
	}
	status += inside;

	/* Top left vertex */
	inside = EVAL_POLY(xl_cur, yt_cur) < 0;
	if (inside ^ (status > 0))
	{
		return Q3C_PARTIAL;
	}
	status += inside;

	if (status == 4)
	{
		/* All the vertices are inside so the square must be covered by
		 * the ellipse */
		return Q3C_COVER;
	}
	else
	{
		/* All the vertices are outside the ellipse
		 * Now we test if the ellipse crosses the edges of the square
		 */
		if (
			(Q3C_INTERSECT(xmin, xmax, xl_cur, xr_cur) &&
			 Q3C_INTERSECT(ymin, ymax, yb_cur, yt_cur)
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
			return Q3C_PARTIAL;
		}
		else
		{
			return Q3C_DISJUNCT;
		}
	}
	#undef POLY_EVAL
}


/* Try to describe the box on the cube face specified by (xmin,xmax,ymin,ymax)
 * as a set of quadtree squares
 * I put them in the stack
 * n0 is the starting resolution
 */
int q3c_setup_square_stack(struct q3c_square *stack, q3c_coord_t xmin,
                           q3c_coord_t ymin, q3c_coord_t xmax, q3c_coord_t ymax,
                           int n0)
{
	/* Here we set up the stack with initial squares */
	int work_nstack = 0;
	q3c_ipix_t ixmin = (Q3C_HALF + xmin) * n0; /* Here I use the C truncation of floats */
	q3c_ipix_t ixmax = (Q3C_HALF + xmax) * n0; /* to integers */
	q3c_ipix_t iymin = (Q3C_HALF + ymin) * n0;
	q3c_ipix_t iymax = (Q3C_HALF + ymax) * n0;
	struct q3c_square *cur_square = stack;

	ixmax = (ixmax == n0 ? (n0 - 1) : ixmax);
	iymax = (iymax == n0 ? (n0 - 1) : iymax);


	if (ixmin == ixmax)
	{
		if (iymin == iymax)
		{
			SET_SQUARE(cur_square, ixmin, iymin, n0);
			work_nstack = 1;
		}
		else
		{
			SET_SQUARE(cur_square, ixmin, iymin, n0);
			cur_square++;
			SET_SQUARE(cur_square, ixmin, iymax, n0);
			work_nstack = 2;
		}
	}
	else
	{
		if (iymin == iymax)
		{
			SET_SQUARE(cur_square, ixmin, iymin, n0);
			cur_square++;
			SET_SQUARE(cur_square, ixmax, iymin, n0);
			work_nstack = 2;
		}
		else
		{
			SET_SQUARE(cur_square, ixmin, iymin, n0);
			cur_square++;
			SET_SQUARE(cur_square, ixmin, iymax, n0);
			cur_square++;
			SET_SQUARE(cur_square, ixmax, iymin, n0);
			cur_square++;
			SET_SQUARE(cur_square, ixmax, iymax, n0);
			work_nstack = 4;
		}
	}
	return work_nstack;
}

/* Process the stack of quad-tree squares and depending on the overlap status
 * I either keep them, expand them into smaller squares or throw them out
 */
void q3c_stack_expand(struct q3c_square* work_stack, int *work_nstack,
                      struct q3c_square* out_stack, int *out_nstack,
                      int cur_depth, int res_depth)
{
	/* Now we select the fully covered set of squares from stack and put them
	 * into out_stack, the partly covered squares are expanded to corresponding
	 * set of 4 squares each.
	 * explanation of the following scheme
	 *    |xxxxxxxxxxxxx     xxxxxxxxxx|
	 *                                 ^
	 *                  /---/           tmp_stack2
	 *                tmp_stack1
	 */
	struct q3c_square *cur_square;
	int tmp_stack1, tmp_stack2;
	q3c_ipix_t xtmp,ytmp,ntmp;
	int j,k;

	for(j = 0, tmp_stack1 = 0, tmp_stack2 = *work_nstack; j < *work_nstack; j++)
	{
		cur_square = work_stack + j;
		//fprintf(stdout,"%d %d %d\n",work_nstack,tmp_stack1,tmp_stack2);
		if (cur_square->status == Q3C_PARTIAL)
		/* If this square partially intersects with the ellipse
		 * I should split this square further
		 */
		{

			/* If this is the last stage of resolution loop, I will not split
			 * the "partial" boxes
			 */
			if (cur_depth == res_depth) continue;
			tmp_stack1++;

			xtmp = 2 * cur_square->x0;
			ytmp = 2 * cur_square->y0;
			ntmp = 2 * cur_square->nside0;

			/* First I try to put the childrens of this square in the part of
			 * the stack freed by thrown away squares (which were disjunct from
			 * the ellipse or which were fully covered by the ellipse)
			 */
			for(k = 0; (k <= 3) && (tmp_stack1 > 0); k++)
			{
				cur_square = work_stack + (j + 1 - tmp_stack1);
				SET_SQUARE(cur_square, xtmp + (k & 1),
				           ytmp + ((k & 2) >> 1), ntmp);
				tmp_stack1--;
			}

			for (; k <= 3; k++)
			{
				cur_square = work_stack + tmp_stack2;
				SET_SQUARE(cur_square, xtmp + (k & 1),
				           ytmp + ((k & 2) >> 1), ntmp);
				tmp_stack2++;
			}

		}
		else
		{
			if (cur_square->status == Q3C_COVER)
			/* I put this square in the output list and
			 * free one place in the stack
			 */
			{
				out_stack[(*out_nstack)++] = *cur_square;
				tmp_stack1++;
			}
			else
			/* This branch can be reached only if status==Q3C_DISJUNCT */
			{
				tmp_stack1++;
				/* I just drop this square and free the place in the stack */
			}
		}

	} /* end of updating of the list of squares loop */

	if (cur_depth == res_depth) return;
	/* After updating the list of squares I compute how many of the
	 * I have them now (except for the case of last resolution step)
	 */
#ifdef Q3C_DEBUG
	fprintf(stdout,"STACK STATE nw_stack: %d nt_stack1: %d nt_stack2: %d\n", *work_nstack, tmp_stack1, tmp_stack2);
#endif

	if (tmp_stack1 == 0)
	{
		*work_nstack = tmp_stack2;
	}
	else
	{
		if ((tmp_stack2-*work_nstack) > tmp_stack1)
		{
			memcpy(work_stack + (*work_nstack - tmp_stack1),
			       work_stack + (tmp_stack2 - tmp_stack1),
			       tmp_stack1 * sizeof(struct q3c_square));
			*work_nstack = tmp_stack2 - tmp_stack1;
		}
		else
		{
			memcpy(work_stack + (*work_nstack - tmp_stack1),
			       work_stack + *work_nstack,
			       (tmp_stack2 - *work_nstack) * sizeof(struct q3c_square));
			*work_nstack = tmp_stack2 - tmp_stack1;
		}
	}
}


/* This function processes the stack of quad-tree squares and puts the
 * fully covered and partially covered squares in the appropriate output
 * lists
 */
void q3c_output_stack( struct q3c_prm *hprm,
                       struct q3c_square *out_stack, int out_nstack,
                       struct q3c_square *work_stack, int work_nstack,
                       int face_num, int nside,
                       q3c_ipix_t *out_ipix_arr_fulls,
                       int *out_ipix_arr_fulls_pos,
                       q3c_ipix_t *out_ipix_arr_partials,
                       int *out_ipix_arr_partials_pos)
{
	int i, j;
	q3c_ipix_t xi, yi, ipix_tmp1, ipix_tmp2, ntmp1;
	q3c_ipix_t  *xbits = hprm->xbits, *ybits = hprm->ybits;

	struct q3c_square *cur_square;
	/* Run through fully covered squares (we take them from out_stack) */
	for(i = 0; i < out_nstack; i++)
	{
		cur_square = out_stack + i;
		ntmp1 = (nside / cur_square->nside0);
		//fprintf(stdout, "XX%lld\n", ntmp1);
		xi = cur_square->x0 * ntmp1;
		yi = cur_square->y0 * ntmp1;

		ipix_tmp1 = q3c_xiyi2ipix(nside, xbits, ybits, face_num, xi, yi);

		ipix_tmp2=ipix_tmp1+(ntmp1*ntmp1);

		/* Now we have in ipix_tmp1 and ipix_tmp2 -- the pixel range for the
		 * query of current square
		 * The query should be     ipix_tmp1 =< II < ipix_tmp2
		 */

		out_ipix_arr_fulls[(*out_ipix_arr_fulls_pos)++] = ipix_tmp1;
		out_ipix_arr_fulls[(*out_ipix_arr_fulls_pos)++] = ipix_tmp2;

	} /* End of output run through fully covered squares */

	if (out_nstack == 0)
	/* If the list of fully covered squares is empty */
	{
		/* Now we just do nothing  -- the stack of ipix'es will be just empty */
	}


	/* Run through partly covered squares (we take them from work_stack where
	 * the cur_square->status == Q3C_PARTIAL)
	 */
	for(i = 0, j = -1; i < work_nstack; i++)
	{
		cur_square = work_stack + i;
		if (cur_square->status != Q3C_PARTIAL)
			continue;
		else
			j+=1;
		ntmp1 = (nside / cur_square->nside0);
		//fprintf(stdout, "XX%lld\n", ntmp1);
		xi = cur_square->x0 * ntmp1;
		yi = cur_square->y0 * ntmp1;

		ipix_tmp1 = q3c_xiyi2ipix(nside, xbits, ybits, face_num, xi, yi);

		ipix_tmp2 = ipix_tmp1 + (ntmp1 * ntmp1);

		/* Now we have in ipix_tmp1 and ipix_tmp2 -- the pixel range for the
		 * query of current square
		 * The query should be     ipix_tmp1 =< II < ipix_tmp2
		 */

		out_ipix_arr_partials[(*out_ipix_arr_partials_pos)++] = ipix_tmp1;
		out_ipix_arr_partials[(*out_ipix_arr_partials_pos)++] = ipix_tmp2;


	} /* End of output run through partly covered squares */

}

static void array_filler(q3c_ipix_t *fulls, int fullpos,
                         q3c_ipix_t *parts, int partpos)
{
	/* We  fill the tail of the out_ipix_arr_fulls and out_ipix_arr_partials
	 * stack by
	 * [1,-1] pairs  since our SQL code wants the arrays of fixed length
	 */

	int i;
	for(i = fullpos; i < (2*Q3C_NFULLS);)
	{
		fulls[i++] = 1;
		fulls[i++] = -1;
	}


	for(i = partpos; i < (2*Q3C_NPARTIALS);)
	{
		parts[i++] = 1;
		parts[i++] = -1;
	}
}


/* Main radial query function */
void q3c_radial_query(struct q3c_prm *hprm, q3c_coord_t ra0,
                      q3c_coord_t dec0, q3c_coord_t rad,
                      q3c_ipix_t *out_ipix_arr_fulls,
                      q3c_ipix_t *out_ipix_arr_partials)
{
	q3c_coord_t axx, ayy, axy, ax, ay, a, xmin, xmax, ymin, ymax,
	            xc_cur = 0, yc_cur = 0, cur_size, xesize, yesize,
	            points[4];

	q3c_ipix_t n0, nside = hprm->nside;

	char face_num, multi_flag = 0, face_count, face_num0, full_flags[3]={0,0,0};
	int out_ipix_arr_fulls_pos = 0;
	int out_ipix_arr_partials_pos = 0;

	int work_nstack = 0, i, j, out_nstack = 0,
	    res_depth;

	struct q3c_square work_stack[Q3C_STACK_SIZE], out_stack[Q3C_STACK_SIZE], *cur_square;

	/* 35 degrees is a magic size above which the cone from the search can
	 * produce a hyperbola or a parabola on a main face and where a lot of
	 * code will start to break.
	 * So if the query is that large, I just query the whole sphere
	 */
	/* TODO
	 * I can instead of querying the whole sphere, just query the appropriate
	 * faces
	 */
	if (rad>=Q3C_MAXRAD)
	{
		q3c_ipix_t maxval = 6*(nside*nside);
		for(i = out_ipix_arr_fulls_pos; i < (2*Q3C_NFULLS);)
		{
			/* don't have any fully covered squares*/
			out_ipix_arr_fulls[i++] = 1;
			out_ipix_arr_fulls[i++] = -1;
		}

		i = out_ipix_arr_partials_pos;
		out_ipix_arr_partials[i++] = -1;
		out_ipix_arr_partials[i++] = maxval;
		/* everything is partially covered */
		for(; i < (2*Q3C_NPARTIALS);)
		{
			/* fill with dummy ranges the rest*/
			out_ipix_arr_partials[i++] = 1;
			out_ipix_arr_partials[i++] = -1;
		}
		return;
	}

	face_num = q3c_get_facenum(ra0, dec0);

	q3c_get_poly_coefs(face_num, ra0, dec0, rad, &axx, &ayy, &axy, &ax, &ay, &a);
	/* The coefficients of the polynome are obtained for the projection
	 * on the cube face for the cube with the edge length 1
	 * axx*x^2+ayy*y^2+axy*x*y+ax*x+ay*y+a
	 */

	q3c_get_xy_minmax(axx, ayy, axy, ax, ay, a, &xmin, &xmax, &ymin, &ymax,
	                  full_flags);

	q3c_multi_face_check(&xmin, &ymin, &xmax, &ymax, points, &multi_flag);

	face_num0 = face_num;

	for(face_count = 0; face_count <= multi_flag; out_nstack = 0, face_count++)
	{
		/* This the beginning of the mega-loop over multiple faces */
		if (face_count > 0)
		/* This "if" works when we pass through the secondary faces */
		{
			face_num = q3c_xy2facenum(2 * points[2 * (face_count - 1)],
			                          2 * points[2 * (face_count - 1) + 1], face_num0);
			q3c_get_poly_coefs(face_num, ra0, dec0, rad, &axx, &ayy, &axy,
			                   &ax, &ay, &a);
			q3c_get_xy_minmax(axx, ayy, axy, ax, ay, a, &xmin, &xmax, &ymin,
			                  &ymax, full_flags+face_count);
		}
		xmax = (xmax > Q3C_HALF ? Q3C_HALF : xmax);
		xmin = (xmin < -Q3C_HALF ? -Q3C_HALF : xmin);
		ymax = (ymax > Q3C_HALF ? Q3C_HALF : ymax);
		ymin = (ymin < -Q3C_HALF ? -Q3C_HALF : ymin);

#ifdef Q3C_DEBUG
		fprintf(stdout,"FACE RUN: %d FACE_NUM: %d\n", face_count, face_num);
#endif
		xesize = xmax - xmin;
		yesize = ymax - ymin;
		xesize = xesize > yesize ? xesize : yesize;

		if (xesize * nside < 1)
		/* If the region is too small */
		{
			xesize = 1 / (q3c_coord_t)nside;
		}

		if (full_flags[(int)face_count])
		/* Take the whole face */
		{

			q3c_ipix_t tmpmin = face_num*nside*nside;
			q3c_ipix_t tmpmax = (face_num+1)*nside*nside;
#ifdef Q3C_DEBUG
			fprintf(stdout, "FULL_FLAG\n");
#endif
			out_ipix_arr_partials[out_ipix_arr_partials_pos++]=tmpmin;
			out_ipix_arr_partials[out_ipix_arr_partials_pos++]=tmpmax;
			continue;
		}

		n0 = 1 << ((q3c_ipix_t)(-q3c_ceil((q3c_log(xesize) / q3c_log(2)))));
		/* n0 is now the level of quadtree for which the minimal
		 * element is >~ our ellipse
		 */

		cur_size = ((q3c_coord_t)1) / n0;
		/* fprintf(stdout, "%lld %Lf %Lf %Lf\n", n0,cur_size,xesize,yesize);*/

#ifdef Q3C_DEBUG
		fprintf(stdout,"XMIN: "Q3C_COORD_FMT " XMAX: "Q3C_COORD_FMT " YMIN: "Q3C_COORD_FMT " YMAX: "Q3C_COORD_FMT "\n", xmin, xmax, ymin, ymax);
#endif

		/* Here we set up the stack with initial squares */

		work_nstack = q3c_setup_square_stack(work_stack,
		                                     xmin, ymin, xmax, ymax, n0);

		/* For this case the maximal increase of resolution of 2^res_depth
		 * for each axis
		 */
		res_depth = nside / n0;
		/* If the the query is too small we cannot go up to Q3C_MAX_DEPTH since we
		 * are limited by nside depth
		 */
		res_depth = Q3C_MAX_DEPTH > res_depth ? res_depth : Q3C_MAX_DEPTH;

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
				xc_cur = (( (q3c_coord_t) cur_square->x0) + Q3C_HALF) / cur_square->nside0 - Q3C_HALF;
				yc_cur = (( (q3c_coord_t) cur_square->y0) + Q3C_HALF) / cur_square->nside0 - Q3C_HALF;
				/* xc_cur and yc_cur -- center of the square (in the coordinate system
				 * of big square [-0.5:0.5]x[-0.5:0.5]
				 */
				//fprintf(stdout,"%Lf %Lf %Lf %d\n",xc_cur,yc_cur,cur_size, status);
				cur_square->status = q3c_circle_cover_check(xc_cur, yc_cur,
				                                            cur_size, xmin, xmax, ymin, ymax,
				                                            axx, axy, ayy, ax, ay, a);

			}

#ifdef Q3C_DEBUG
			fprintf(stdout,"2) NUM squares in the stack %d\n",work_nstack);
#endif

			q3c_stack_expand(work_stack, &work_nstack,
			                 out_stack, &out_nstack,
			                 i, res_depth);


		} /* end of resolution loop */

		//   Old printing of the results
#ifdef Q3C_DEBUG
		for(i = 0; i < out_nstack; i++)
		{
			cur_square = out_stack + i;
			fprintf(stdout, "OUT: %f %f %d %d\n", cur_square->x0+0.5,cur_square->y0+0.5,cur_square->nside0,cur_square->status);
		}

		for(i = 0; i < work_nstack; i++)
		{
			cur_square = work_stack + i;
			if (cur_square->status == Q3C_PARTIAL)
				fprintf(stdout, "OUT1: %f %f %d %d\n", cur_square->x0+0.5,cur_square->y0+0.5,cur_square->nside0,cur_square->status);
		}
#endif

		q3c_output_stack( hprm, out_stack, out_nstack,
		                  work_stack, work_nstack,
		                  face_num, nside,
		                  out_ipix_arr_fulls,
		                  &out_ipix_arr_fulls_pos,
		                  out_ipix_arr_partials,
		                  &out_ipix_arr_partials_pos);


	} /* End of the mega-loop over the faces */


	array_filler(out_ipix_arr_fulls, out_ipix_arr_fulls_pos,
	             out_ipix_arr_partials, out_ipix_arr_partials_pos);

} /* End of q3c_radial_query() */



void q3c_poly_query(struct q3c_prm *hprm, q3c_poly *qp,
                    q3c_ipix_t *out_ipix_arr_fulls,
                    q3c_ipix_t *out_ipix_arr_partials,
                    char *too_large)
{


	q3c_coord_t xmin, xmax, ymin, ymax,
	            xc_cur = 0, yc_cur = 0, cur_size, xesize, yesize,
	            points[4];

	q3c_ipix_t n0, nside = hprm->nside;

	char face_num, multi_flag = 0, face_count, face_num0, large_flag = 0;
	int out_ipix_arr_fulls_pos = 0;
	int out_ipix_arr_partials_pos = 0;

	int work_nstack = 0, i, j, out_nstack = 0, res_depth;

	struct q3c_square work_stack[Q3C_STACK_SIZE], out_stack[Q3C_STACK_SIZE], *cur_square;

	face_num = q3c_get_facenum_poly(qp);

	q3c_project_poly(qp, face_num, &large_flag);
	if (large_flag)
	{
		*too_large = 1;
		return;
	}
	q3c_prepare_poly(qp);

	q3c_get_minmax_poly(qp, &xmin, &xmax, &ymin, &ymax);


	q3c_multi_face_check(&xmin, &ymin, &xmax, &ymax, points, &multi_flag);


	face_num0 = face_num;

	for(face_count = 0; face_count <= multi_flag; out_nstack = 0, face_count++)
	{
		/* This the beginning of the mega-loop over multiple faces */

		if (face_count > 0)
		/* This "if" works when we pass through the secondary faces */
		{
			face_num = q3c_xy2facenum(2 * points[2 * (face_count - 1)],
			                          2 * points[2 * (face_count - 1) + 1], face_num0);

			q3c_project_poly(qp, face_num, &large_flag);
			if (large_flag)
			{
				*too_large = 1;
				return;
			}

			q3c_prepare_poly(qp);

			q3c_get_minmax_poly(qp, &xmin, &xmax, &ymin, &ymax);

			xmax = (xmax > Q3C_HALF ? Q3C_HALF : xmax);
			xmin = (xmin < -Q3C_HALF ? -Q3C_HALF : xmin);
			ymax = (ymax > Q3C_HALF ? Q3C_HALF : ymax);
			ymin = (ymin < -Q3C_HALF ? -Q3C_HALF : ymin);
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
		fprintf(stdout,"XMIN: "Q3C_COORD_FMT " XMAX: "Q3C_COORD_FMT " YMIN: "Q3C_COORD_FMT " YMAX: "Q3C_COORD_FMT "\n", xmin, xmax, ymin, ymax);
#endif

		work_nstack = q3c_setup_square_stack(work_stack,
		                                     xmin, ymin, xmax, ymax, n0);


		/* For this case the maximal increase of resolution of 2^res_depth
		 * for each axis
		 */

		res_depth = nside / n0;
		/* If the the query is too small we cannot go up to Q3C_MAX_DEPTH since we
		 * are limited by nside depth
		 */
		res_depth = Q3C_MAX_DEPTH > res_depth ? res_depth : Q3C_MAX_DEPTH;

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
				xc_cur = (( (q3c_coord_t) cur_square->x0) + Q3C_HALF) / cur_square->nside0 - Q3C_HALF;
				yc_cur = (( (q3c_coord_t) cur_square->y0) + Q3C_HALF) / cur_square->nside0 - Q3C_HALF;
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
			q3c_stack_expand(work_stack, &work_nstack,
			                 out_stack, &out_nstack,
			                 i, res_depth);
		}

		//   Old printing of the results
#ifdef Q3C_DEBUG
		for(i = 0; i < out_nstack; i++)
		{
			cur_square = out_stack + i;
			fprintf(stdout, "OUT: %f %f %d %d\n", cur_square->x0+0.5,cur_square->y0+0.5,cur_square->nside0,cur_square->status);
		}

		for(i = 0; i < work_nstack; i++)
		{
			cur_square = work_stack + i;
			if (cur_square->status == Q3C_PARTIAL)
				fprintf(stdout, "OUT1: %f %f %d %d\n", cur_square->x0+0.5,cur_square->y0+0.5,cur_square->nside0,cur_square->status);
		}
#endif

		q3c_output_stack( hprm, out_stack, out_nstack,
		                  work_stack, work_nstack,
		                  face_num, nside,
		                  out_ipix_arr_fulls,
		                  &out_ipix_arr_fulls_pos,
		                  out_ipix_arr_partials,
		                  &out_ipix_arr_partials_pos);


	} /* End of the mega-loop over the faces */


	array_filler(out_ipix_arr_fulls, out_ipix_arr_fulls_pos,
	             out_ipix_arr_partials, out_ipix_arr_partials_pos);

} /* End of radial_query() */





void q3c_ellipse_query(struct q3c_prm *hprm, q3c_coord_t ra0,
                       q3c_coord_t dec0, q3c_coord_t majax, q3c_coord_t ell,
                       q3c_coord_t PA, q3c_ipix_t *out_ipix_arr_fulls,
                       q3c_ipix_t *out_ipix_arr_partials)
{
	q3c_coord_t xmin, xmax, ymin, ymax, xc_cur = 0,
	            yc_cur = 0, cur_size, xesize, yesize,
	            points[4], axx, ayy, axy, ax, ay, a;

	q3c_ipix_t n0, nside = hprm->nside;

	char face_num, multi_flag = 0, face_count, face_num0;
	int out_ipix_arr_fulls_pos = 0;
	int out_ipix_arr_partials_pos = 0;

	int work_nstack = 0, i, j, out_nstack = 0,
	    res_depth;

	struct q3c_square work_stack[Q3C_STACK_SIZE], out_stack[Q3C_STACK_SIZE], *cur_square;

	/* 35 degrees is a magic size above which the cone from the search can
	 * produce a hyperbola or a parabola on a main face and where a lot of
	 * code will start to break.
	 * So if the query is that large, I just query the whole sphere
	 */
	/* TODO
	 * I can instead of querying the whole sphere, just query the appropriate
	 * faces
	 */
	if (majax>= Q3C_MAXRAD)
	{
		q3c_ipix_t maxval = 6*(nside*nside);
		for(i = out_ipix_arr_fulls_pos; i < (2*Q3C_NFULLS);)
		{
			/* don't have any fully covered squares*/
			out_ipix_arr_fulls[i++] = 1;
			out_ipix_arr_fulls[i++] = -1;
		}

		i = out_ipix_arr_partials_pos;
		out_ipix_arr_partials[i++] = -1;
		out_ipix_arr_partials[i++] = maxval;
		/* everything is partially covered */
		for(; i < (2*Q3C_NPARTIALS);)
		{
			/* fill with dummy ranges the rest*/
			out_ipix_arr_partials[i++] = 1;
			out_ipix_arr_partials[i++] = -1;
		}
		return;
	}

	face_num = q3c_get_facenum(ra0, dec0);

	q3c_fast_get_ellipse_xy_minmax_and_poly_coefs(face_num, ra0, dec0, majax,
	                                              ell, PA, &xmin, &xmax, &ymin, &ymax, &axx, &ayy, &axy, &ax, &ay, &a);

	q3c_multi_face_check(&xmin, &ymin, &xmax, &ymax, points, &multi_flag);

	face_num0 = face_num;

	for(face_count = 0; face_count <= multi_flag; out_nstack = 0, face_count++)
	{
		/* This the beginning of the mega-loop over multiple faces */
		if (face_count > 0)
		/* This "if" works when we pass through the secondary faces */
		{
			face_num = q3c_xy2facenum(2 * points[2 * (face_count - 1)],
			                          2 * points[2 * (face_count - 1) + 1], face_num0);

			q3c_fast_get_ellipse_xy_minmax_and_poly_coefs(face_num, ra0, dec0, majax,
			                                              ell, PA, &xmin, &xmax, &ymin, &ymax, &axx, &ayy, &axy, &ax, &ay, &a);

			xmax = (xmax > Q3C_HALF ? Q3C_HALF : xmax);
			xmin = (xmin < -Q3C_HALF ? -Q3C_HALF : xmin);
			ymax = (ymax > Q3C_HALF ? Q3C_HALF : ymax);
			ymin = (ymin < -Q3C_HALF ? -Q3C_HALF : ymin);
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
#ifdef Q3C_DEBUG
		fprintf(stdout,"XMIN: "Q3C_COORD_FMT " XMAX: "Q3C_COORD_FMT " YMIN: "Q3C_COORD_FMT " YMAX: "Q3C_COORD_FMT "\n", xmin, xmax, ymin, ymax);
		fprintf(stdout,Q3C_COORD_FMT " "Q3C_COORD_FMT " "Q3C_COORD_FMT " "Q3C_COORD_FMT " "Q3C_COORD_FMT " "Q3C_COORD_FMT "\n", axx,ayy,axy,ax,ay,a);
#endif

		work_nstack = q3c_setup_square_stack(work_stack,
		                                     xmin, ymin, xmax, ymax, n0);

		/* For this case the maximal increase of resolution of 2^res_depth
		 * for each axis
		 */

		res_depth = nside / n0;
		/* If the the query is too small we cannot go up to Q3C_MAX_DEPTH since we
		 * are limited by nside depth
		 */
		res_depth = Q3C_MAX_DEPTH > res_depth ? res_depth : Q3C_MAX_DEPTH;

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
				xc_cur = (( (q3c_coord_t) cur_square->x0) + Q3C_HALF) / cur_square->nside0 - Q3C_HALF;
				yc_cur = (( (q3c_coord_t) cur_square->y0) + Q3C_HALF) / cur_square->nside0 - Q3C_HALF;
				/* xc_cur and yc_cur -- center of the square (in the coordinate system
				 * of big square [-0.5:0.5]x[-0.5:0.5]
				 */
				//fprintf(stdout,"%Lf %Lf %Lf %d\n",xc_cur,yc_cur,cur_size, status);
				cur_square->status = q3c_circle_cover_check(xc_cur, yc_cur,
				                                            cur_size, xmin, xmax, ymin, ymax, axx, axy, ayy, ax, ay, a);
#ifdef Q3C_DEBUG
				fprintf(stderr,"Trying (%f %f %f)(%f %f %f %f) --- ",xc_cur,
				        yc_cur,cur_size,xc_cur-cur_size/2,xc_cur+cur_size/2,yc_cur-cur_size/2,yc_cur+cur_size/2);
				fprintf(stderr,"%d\n",cur_square->status);
#endif
			}

#ifdef Q3C_DEBUG
			fprintf(stdout,"2) NUM squares in the stack %d\n",work_nstack);
#endif

			q3c_stack_expand(work_stack, &work_nstack,
			                 out_stack, &out_nstack,
			                 i, res_depth);


		} /* end of resolution loop */

		//   Old printing of the results
#ifdef Q3C_DEBUG
		for(i = 0; i < out_nstack; i++)
		{
			cur_square = out_stack + i;
			fprintf(stdout, "OUT: %f %f %d %d\n", cur_square->x0+0.5,cur_square->y0+0.5,cur_square->nside0,cur_square->status);
		}

		for(i = 0; i < work_nstack; i++)
		{
			cur_square = work_stack + i;
			if (cur_square->status == Q3C_PARTIAL)
				fprintf(stdout, "OUT1: %f %f %d %d\n", cur_square->x0+0.5,cur_square->y0+0.5,cur_square->nside0,cur_square->status);
		}
#endif

		q3c_output_stack( hprm, out_stack, out_nstack,
		                  work_stack, work_nstack,
		                  face_num, nside,
		                  out_ipix_arr_fulls,
		                  &out_ipix_arr_fulls_pos,
		                  out_ipix_arr_partials,
		                  &out_ipix_arr_partials_pos);


	} /* End of the mega-loop over the faces */


	array_filler(out_ipix_arr_fulls, out_ipix_arr_fulls_pos,
	             out_ipix_arr_partials, out_ipix_arr_partials_pos);

} /* End of q3c_ellipse_query() */

