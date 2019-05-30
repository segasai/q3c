/*
	   Copyright (C) 2004-2015 Sergey Koposov

    Email: koposov@ast.cam.ac.uk

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
/* Implementation of the crossing algorithm */
{
	int i, n = qp->n;
	q3c_coord_t *y = qp->y;
	q3c_coord_t *x = qp->x;
	q3c_coord_t *ax = qp->ax;
	q3c_coord_t *ay = qp->ay;
	int result = !Q3C_DISJUNCT;
	for(i=0;i<n;i++)
	{
		if (((y0<=y[i])==(y0>y[(i+1)%n])) &&
				((x0-x[i])<(y0-y[i])*ax[i]/ay[i]))
		{
			result=!result;
		}
	}
	return !result;
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

void q3c_project_poly(q3c_poly *qp, char face_num, char *large_flag)
{
	q3c_coord_t ra1, dec1, tmp0;
	q3c_coord_t *ra = qp->ra, *dec = qp->dec;
	q3c_coord_t *x = qp->x, *y = qp->y, x0, y0;
	q3c_coord_t tmpval ;
	int i, n = qp->n;
	if ((face_num > 0) && (face_num < 5))
	{
		face_num--; /* Just computation trick */
		for (i = 0; i < n; i++)
		{
			ra1 = Q3C_DEGRA * (ra[i] - 90 * (q3c_coord_t)face_num);
			dec1 = Q3C_DEGRA * dec[i];
			tmpval =  q3c_cos(ra1);
			if (tmpval < Q3C_MINDISCR)
			{
				*large_flag = 1;
			}
			x[i] = (q3c_tan(ra1)) / 2;
			y[i] = (q3c_tan(dec1)) / tmpval / 2;
		}
		/* Now x[i] and y[i] are coordinates on cube face [-0.5:0.5]x[-0.5:0.5] */
	}
	else if (face_num == 0)
	{
		for (i = 0; i < n; i++)
		{
			ra1 = Q3C_DEGRA * ra[i];
			dec1 = Q3C_DEGRA * dec[i];
			tmpval = q3c_tan(dec1);
			if (tmpval < Q3C_MINDISCR)
			{
				*large_flag = 1;
			}
			tmp0 = 1 / tmpval;
			q3c_sincos(ra1, x0, y0);

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
			ra1 = Q3C_DEGRA * ra[i];
			dec1 = Q3C_DEGRA * dec[i];
			tmpval = q3c_tan(dec1);
			if (tmpval > -Q3C_MINDISCR)
			{
				*large_flag = 1;
			}
			tmp0 = 1 / tmpval;

			q3c_sincos(ra1, x0, y0);

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
	if (val != Q3C_DISJUNCT)
	{
		goto PARTUNDEF_CHECK01;
	}

	val = q3c_check_point_in_poly(qp, xr_cur, yb_cur);
	if (val != Q3C_DISJUNCT)
	{
		return Q3C_PARTIAL;
	}

	val = q3c_check_point_in_poly(qp, xr_cur, yt_cur);
	if (val != Q3C_DISJUNCT)
	{
		return Q3C_PARTIAL;
	}

	val = q3c_check_point_in_poly(qp, xl_cur, yt_cur);

	if (val != Q3C_DISJUNCT)
	{
		return Q3C_PARTIAL;
	}
	else
	{
		if (q3c_poly_intersection_check(qp, xl_cur, xr_cur, yb_cur, yt_cur, cur_size)||
			((qp->x[0] > xl_cur) && (qp->x[0] < xr_cur) &&
			(qp->y[0] > yb_cur) && (qp->y[0] < yt_cur)))
		{
			return Q3C_PARTIAL;
		}
		else
		{
			return Q3C_DISJUNCT;
		}
	}


	PARTUNDEF_CHECK01:
	val = q3c_check_point_in_poly(qp, xr_cur, yb_cur);
	if (val == Q3C_DISJUNCT)
	{
		return Q3C_PARTIAL;
	}

	//PARTUNDEF_CHECK11:
	val = q3c_check_point_in_poly(qp, xr_cur, yt_cur);
	if (val == Q3C_DISJUNCT)
	{
		return Q3C_PARTIAL;
	}


	//PARTUNDEF_CHECK10:
	val = q3c_check_point_in_poly(qp, xl_cur, yt_cur);
	if (val == Q3C_DISJUNCT)
	{
		return Q3C_PARTIAL;
	}
	else
	{
		return Q3C_COVER;
	}
}


int q3c_check_sphere_point_in_poly(struct q3c_prm *hprm, int n,
                                   q3c_coord_t in_ra[], q3c_coord_t in_dec[],
                                   q3c_coord_t ra0, q3c_coord_t dec0,
                                   char *too_large,
                                   int invocation,
																 	 q3c_coord_t (*xpj)[Q3C_MAX_N_POLY_VERTEX],
																	 q3c_coord_t (*ypj)[Q3C_MAX_N_POLY_VERTEX],
																 	 q3c_coord_t (*axpj)[Q3C_MAX_N_POLY_VERTEX],
																	 q3c_coord_t (*aypj)[Q3C_MAX_N_POLY_VERTEX],
																   char *faces, char *multi_flag)
{
	q3c_coord_t xmin,xmax,ymin, ymax;
	q3c_ipix_t ipix;
	q3c_coord_t points[4];
	char face_num, face_num0, cur_face_num, large_flag=0;
	q3c_coord_t x0, y0;

	int face_count = -1, i;

	q3c_poly qp;

	q3c_ang2ipix_xy(hprm, ra0, dec0, &cur_face_num, &ipix, &x0, &y0);

	qp.ra = in_ra;
	qp.dec = in_dec;
	qp.n = n;

	if (invocation == 0)
	{
		face_num = q3c_get_facenum_poly(&qp);
		faces[0] = face_num;

		qp.x = xpj[0];
		qp.y = ypj[0];
		qp.ax = axpj[0];
		qp.ay = aypj[0];

		q3c_project_poly(&qp, face_num, &large_flag);
		if (large_flag)
		{
			*too_large=1;
		}
		q3c_prepare_poly(&qp);

		q3c_get_minmax_poly(&qp, &xmin, &xmax, &ymin, &ymax);

		/* Now I determine whether the poly
		 * intersect other faces or not, and if yes, I setup the array "points" to the
		 * multi_face loop.
		 */

		 q3c_multi_face_check(&xmin, &ymin, &xmax, &ymax, points, multi_flag);


		face_num0 = face_num;

		for(face_count = 0; face_count <= *multi_flag; face_count++)
		{
		/* This the beginning of the mega-loop over multiple faces */

			if (face_count > 0)
			/* This "if" works when we pass through the secondary faces */
			{
				face_num = q3c_xy2facenum(2 * points[2 * face_count - 2],
					2 * points[2 * face_count - 1], face_num0);

				faces[face_count] = face_num;

				qp.x = xpj[face_count];
				qp.y = ypj[face_count];
				qp.ax = axpj[face_count];
				qp.ay = aypj[face_count];

				q3c_project_poly(&qp, faces[face_count], &large_flag);
				if (large_flag)
				{
					*too_large=1;
				}

				q3c_prepare_poly(&qp);
			}
		}
	}

	for (i = 0; i <= *multi_flag; i++)
	{
		if (faces[i] == cur_face_num)
		{
			face_count = i;
			break;
		}
	}

	if (i == (*multi_flag + 1))
	{
		return 0;
	}

	qp.x = xpj[face_count];
	qp.y = ypj[face_count];
	qp.ax = axpj[face_count];
	qp.ay = aypj[face_count];

	return q3c_check_point_in_poly(&qp, x0, y0);
}
