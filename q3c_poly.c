/*
       Copyright (C) 2004-2026 Sergey Koposov

    Email:  skoposov AT ed DOT ac DOT uk

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

void q3c_prepare_poly(q3c_poly *qp)
{
    /* Compute the sides of the polygon for a given projection */
    int n = qp->n - 1;
    int i;
    q3c_coord_t *ax = qp->ax;
    q3c_coord_t *ay = qp->ay;
    q3c_coord_t *x = qp->x;
    q3c_coord_t *y = qp->y;

    for (i = 0; i < n; i++)
    {
        ax[i] = x[i + 1] - x[i];
        ay[i] = y[i + 1] - y[i];
    }
    ax[i] = x[0] - x[i];
    ay[i] = y[0] - y[i];
}

int q3c_check_point_in_poly(q3c_poly *qp, q3c_coord_t x0, q3c_coord_t y0)
/* Implementation of the crossing algorithm */
{
    int i, n = qp->n;
    q3c_coord_t *y = qp->y;
    q3c_coord_t *x = qp->x;
    q3c_coord_t *ax = qp->ax;
    q3c_coord_t *ay = qp->ay;
    int result = !Q3C_DISJUNCT;
    for (i = 0; i < n; i++)
    {
        if (((y0 <= y[i]) == (y0 > y[(i + 1) % n])) && ((x0 - x[i]) < (y0 - y[i]) * ax[i] / ay[i]))
        {
            result = !result;
        }
    }
    return !result;
}

void q3c_get_minmax_poly(q3c_poly *qp, q3c_coord_t *xmin, q3c_coord_t *xmax, q3c_coord_t *ymin, q3c_coord_t *ymax)
{
    /* Compute the bounding box of the polygon in the current face */
    int i;
    const int n = qp->n;
    q3c_coord_t *x = qp->x, *y = qp->y, t;
    q3c_coord_t xmi, xma, ymi, yma;

    xmi = x[0];
    xma = x[0];
    ymi = y[0];
    yma = y[0];

    for (i = 1; i < n; i++)
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

/* Clip the spherical polygon (n vertices ra, dec in degrees, joined by great
 * circle arcs) to each of the six face pyramids and project the pieces onto
 * their faces (COORDINATES.md section 3).
 * The polygon must lie within an open hemisphere: then its inside is the side
 * which does not contain the opposite hemisphere, and the piece on face f is
 * the polygon clipped by the four planes |t1| <= t0, |t2| <= t0, where
 * t = (v.n_f, v.u_f, v.w_f). The clipping is done in 3D (Sutherland-Hodgman,
 * interpolating linearly along the chords, which project onto the great
 * circle arcs), so it is exact and needs no projection of the vertices far
 * from the face. The pyramids are enlarged by a relative 1e-12 so that a
 * point on a face edge is never outside the piece of its own face because of
 * rounding.
 * Returns 1 if the polygon does not lie within an open hemisphere (or, which
 * should not happen, a piece does not fit in Q3C_MAX_N_FACE_VERTEX vertices).
 */
char q3c_poly_to_faces(int n, const q3c_coord_t *ra, const q3c_coord_t *dec, q3c_poly_faces *pf)
{
    const q3c_coord_t lim = 1 + 1e-12;
    q3c_coord_t v[Q3C_MAX_N_POLY_VERTEX][3], m[3] = {0, 0, 0}, buf[2][Q3C_MAX_N_FACE_VERTEX][3], sa, ca, sd, cd;
    int i, j, k, f, iter, nin, nout, cur;
    q3c_poly qp;

    for (i = 0; i < n; i++)
    {
        q3c_sincos(ra[i] * Q3C_DEGRA, sa, ca);
        q3c_sincos(dec[i] * Q3C_DEGRA, sd, cd);
        v[i][0] = cd * ca, v[i][1] = cd * sa, v[i][2] = sd;
    }
    /* look for m with v_i.m > 0 for all the vertices (perceptron algorithm,
     * which finds it in a finite number of steps if it exists) */
    for (iter = 0;; iter++)
    {
        for (i = 0; i < n && (v[i][0] * m[0] + v[i][1] * m[1] + v[i][2] * m[2]) > 0; i++)
            ;
        if (i == n)
        {
            break;
        }
        if (iter == 10 * Q3C_MAX_N_POLY_VERTEX)
        {
            return 1;
        }
        for (j = 0; j < 3; j++)
        {
            m[j] += v[i][j];
        }
    }

    for (f = 0; f < 6; f++)
    {
        for (i = 0; i < n; i++)
        {
            q3c_to_face_frame(f, v[i], buf[0][i]);
        }
        /* clip by the half-spaces lim t0 + t1 >= 0, lim t0 - t1 >= 0, then the same with t2 */
        for (k = 0, cur = 0, nin = n; k < 4; k++, cur = 1 - cur, nin = nout)
        {
            const int jj = 1 + k / 2;
            const q3c_coord_t sgn = (k % 2) ? -1 : 1;
            q3c_coord_t(*in)[3] = buf[cur], (*out)[3] = buf[1 - cur];

            for (i = 0, nout = 0; i < nin; i++)
            {
                const q3c_coord_t *a = in[i], *b = in[(i + 1) % nin];
                const q3c_coord_t da = lim * a[0] + sgn * a[jj], db = lim * b[0] + sgn * b[jj];

                if (nout + 2 > Q3C_MAX_N_FACE_VERTEX)
                {
                    return 1;
                }
                if (da >= 0)
                {
                    for (j = 0; j < 3; j++)
                    {
                        out[nout][j] = a[j];
                    }
                    nout++;
                }
                if ((da >= 0) != (db >= 0))
                /* the edge crosses the plane */
                {
                    for (j = 0; j < 3; j++)
                    {
                        out[nout][j] = a[j] + (b[j] - a[j]) * (da / (da - db));
                    }
                    nout++;
                }
            }
        }
        /* fewer than 3 vertices means the polygon only touches the face */
        pf->n[f] = (nin >= 3) ? nin : 0;
        for (i = 0; i < pf->n[f]; i++)
        {
            pf->x[f][i] = buf[cur][i][1] / buf[cur][i][0];
            pf->y[f][i] = buf[cur][i][2] / buf[cur][i][0];
        }
        q3c_face_poly(pf, f, &qp);
        if (qp.n > 0)
        {
            q3c_prepare_poly(&qp);
        }
    }
    return 0;
}

/* The piece of the polygon on face face_num */
void q3c_face_poly(q3c_poly_faces *pf, char face_num, q3c_poly *qp)
{
    qp->n = pf->n[(int)face_num];
    qp->x = pf->x[(int)face_num];
    qp->y = pf->y[(int)face_num];
    qp->ax = pf->ax[(int)face_num];
    qp->ay = pf->ay[(int)face_num];
}

static char q3c_poly_intersection_check(q3c_poly *qp, q3c_coord_t xl, q3c_coord_t xr, q3c_coord_t yb, q3c_coord_t yt,
                                        q3c_coord_t cur_size)
{
    int i, n = qp->n;
    q3c_coord_t *ax = qp->ax;
    q3c_coord_t *ay = qp->ay;
    q3c_coord_t *x = qp->x;
    q3c_coord_t *y = qp->y;
    q3c_coord_t txl, txr, tyb, tyt, axi, ayi, xi, yi, tmp, tmp1;
    char ret = 0;
    for (i = 0; i < n; i++)
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

/* Four corners inside => Q3C_COVER: a concave notch can cut into such a square, so fulls need the q3c_in_poly() recheck */
int q3c_poly_cover_check(q3c_poly *qp, q3c_coord_t xc_cur, q3c_coord_t yc_cur, q3c_coord_t cur_size)
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
        if (q3c_poly_intersection_check(qp, xl_cur, xr_cur, yb_cur, yt_cur, cur_size) ||
            ((qp->x[0] > xl_cur) && (qp->x[0] < xr_cur) && (qp->y[0] > yb_cur) && (qp->y[0] < yt_cur)))
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

    // PARTUNDEF_CHECK11:
    val = q3c_check_point_in_poly(qp, xr_cur, yt_cur);
    if (val == Q3C_DISJUNCT)
    {
        return Q3C_PARTIAL;
    }

    // PARTUNDEF_CHECK10:
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

/* Check whether the point (ra0, dec0) is inside the polygon (n vertices ra,
 * dec). With invocation == 0 the polygon is clipped to the faces and stored
 * in pf, otherwise pf from a previous call with the same polygon is used.
 * *too_large is set if the polygon does not lie within a hemisphere.
 */
int q3c_check_sphere_point_in_poly(struct q3c_prm *hprm, int n, const q3c_coord_t *ra, const q3c_coord_t *dec,
                                   q3c_coord_t ra0, q3c_coord_t dec0, char *too_large, int invocation,
                                   q3c_poly_faces *pf)
{
    q3c_coord_t X, Y;
    q3c_ipix_t ipix;
    char face_num;
    q3c_poly qp;

    if (invocation == 0 && q3c_poly_to_faces(n, ra, dec, pf))
    {
        *too_large = 1;
        return Q3C_DISJUNCT;
    }
    q3c_ang2ipix_xy(hprm, ra0, dec0, &face_num, &ipix, &X, &Y);
    q3c_face_poly(pf, face_num, &qp);
    if (qp.n == 0)
    {
        return Q3C_DISJUNCT;
    }
    return q3c_check_point_in_poly(&qp, X, Y);
}
