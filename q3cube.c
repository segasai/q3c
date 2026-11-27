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
#include "my_bits.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static char __q3c_version[] = Q3C_VERSION;

/* Naming of the coordinates follows COORDINATES.md: X, Y in [-1, 1] are the
 * face coordinates (named X, Y where they could be confused with the
 * Cartesian x, y, z, and xmin, x0 etc. in the query code), xi, yi are the
 * integer coordinates of the finest cells and (ix, iy) those of the cells of
 * a coarser level n0.
 */

/* Second order curve axx dx^2 + ayy dy^2 + axy dx dy + ax dx + ay dy + a = 0,
 * negative inside the region, in the face coordinates relative to the
 * projection (x0, y0) of the region centre: dx = X - x0, dy = Y - y0.
 * Near a small region all the terms are then of the same order and the
 * curve can be evaluated without cancellations.
 */
typedef struct
{
    q3c_coord_t x0, y0, axx, ayy, axy, ax, ay, a;
} q3c_conic;

static char q3c_face_setup(char face_num, q3c_region region, void *region_data, q3c_coord_t *xmin, q3c_coord_t *xmax,
                           q3c_coord_t *ymin, q3c_coord_t *ymax, q3c_conic *c, char *full_flag);

void q3c_get_version(char *out, int maxchar)
{
    strncpy(out, __q3c_version, maxchar);
}

/* Distance calculation routine, inputs and outputs are in degrees */
q3c_coord_t q3c_dist(q3c_coord_t ra1, q3c_coord_t dec1, q3c_coord_t ra2, q3c_coord_t dec2)
{
    return 2 * q3c_asin(q3c_sqrt(q3c_sindist(ra1, dec1, ra2, dec2))) * Q3C_RADEG;
}

/* sin(Distance/2)^2 calculation routine, inputs and outputs are in degrees */
q3c_coord_t q3c_sindist(q3c_coord_t ra1, q3c_coord_t dec1, q3c_coord_t ra2, q3c_coord_t dec2)
{
    /* Fast and precise way to compute the distance on the sphere
     * it uses just 3 evaluations of trigonometric functions
     */
    q3c_coord_t x, y, z;
    x = q3c_sin((ra1 - ra2) / 2 * Q3C_DEGRA);
    x *= x;
    y = q3c_sin((dec1 - dec2) / 2 * Q3C_DEGRA);
    y *= y;

    /* Seem to be more precise :) */
    z = q3c_cos((dec1 + dec2) / 2 * Q3C_DEGRA);
    z *= z;

    return x * (z - y) + y;
}

/* (ra, dec) in degrees, 0<=ra<360 -> face number and the face coordinates
 * X, Y in [-1, 1] (COORDINATES.md section 3). The equatorial face is
 * selected by ra first and replaced by a polar one if |Y| > 1 there.
 */
static char q3c_ang2XY(q3c_coord_t ra, q3c_coord_t dec, q3c_coord_t *X, q3c_coord_t *Y)
{
    char face_num = q3c_fmod((ra + 45) / 90, 4);
    const q3c_coord_t ra1 = Q3C_DEGRA * (ra - 90 * (q3c_coord_t)face_num), tan_dec = q3c_tan(Q3C_DEGRA * dec);
    q3c_coord_t inv_tan_dec;

    *X = q3c_tan(ra1);
    *Y = tan_dec / q3c_cos(ra1);
    if (!(*Y > 1 || *Y < -1))
    {
        return face_num + 1;
    }
    /* (X, Y) = cot(dec) (sin ra, -cos ra) on face 0 and
     * (X, Y) = -cot(dec) (sin ra, cos ra) on face 5
     */
    face_num = (*Y > 1) ? 0 : 5;
    inv_tan_dec = 1 / tan_dec;
    q3c_sincos(Q3C_DEGRA * ra, *X, *Y);
    *X *= (face_num == 0 ? inv_tan_dec : -inv_tan_dec);
    *Y *= -inv_tan_dec;
    return face_num;
}

/* convert angular coordinates (ra,dec) -> ipix
 * ang2ipix is also outputting the face coordinates X, Y in [-1,1]
 * ipix structure is the following:
 * Bit 0(leftmost) : unused
 * Bit 1-3: Cube face id (0 is the top face, 5 is  the bottom face,
 * 1-4 are faces looking towards (x,y)=(1,0), (0,1), (-1,0), (0,-1)
 * Bit 4-63: 60-bit long location in the quadtree on the cube face,
 * encoded by z-order (e.g. two interleaved bit strings on for x,
 * another for y)
 * See COORDINATES.md for the details
 */
void q3c_ang2ipix_xy(struct q3c_prm *hprm, q3c_coord_t ra0, q3c_coord_t dec0, char *out_face_num, q3c_ipix_t *ipix,
                     q3c_coord_t *X_out, q3c_coord_t *Y_out)
/* ra in degrees, dec in degrees, out of range values are wrapped/clamped */
{
    const q3c_ipix_t nside = hprm->nside;
    q3c_coord_t X, Y;
    q3c_ipix_t xi, yi;
    char face_num = q3c_ang2XY(UNWRAP_RA(ra0), dec0 > 90 ? 90 : (dec0 < -90 ? -90 : dec0), &X, &Y);

    *X_out = X;
    *Y_out = Y;

    /* integer cell coordinates (COORDINATES.md section 4); the X = 1 and
     * Y = 1 edges go to the last cells */
    xi = (q3c_ipix_t)(((X + 1) / 2) * nside);
    yi = (q3c_ipix_t)(((Y + 1) / 2) * nside);
    if (xi == nside)
    {
        xi--;
    }
    if (yi == nside)
    {
        yi--;
    }

    *ipix = q3c_xiyi2ipix(nside, hprm->xbits, hprm->ybits, face_num, xi, yi);
    *out_face_num = face_num;
}

/* convert coordinates (ra,dec) -> ipix
 * ra, dec in degrees
 * and strictly 0<=ra<360 and -90<=dec<=90
 */
void q3c_ang2ipix(struct q3c_prm *hprm, q3c_coord_t ra0, q3c_coord_t dec0, q3c_ipix_t *ipix)
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
    q3c_coord_t X, Y;

    if (dec >= 90)
    /* Poles */
    {
        return 0;
    }
    else if (dec <= -90)
    {
        return 5;
    }
    return q3c_ang2XY(ra, dec, &X, &Y);
}

/* get the main cube face number for a given region
 * CIRCLE/ELLIPSE/POLYGON
 */
char q3c_get_region_facenum(q3c_region region, void *data)
{
    switch (region)
    {
    case Q3C_CIRCLE: {
        return q3c_get_facenum(((q3c_circle_region *)data)->ra, ((q3c_circle_region *)data)->dec);
    }
    case Q3C_ELLIPSE: {
        return q3c_get_facenum(((q3c_ellipse_region *)data)->ra, ((q3c_ellipse_region *)data)->dec);
    }
    case Q3C_POLYGON: {
        return q3c_get_facenum_poly((q3c_poly *)data);
    }
    default:
        return 1;
    }
}

/* radius (semi-major axis for ellipses) of a CIRCLE/ELLIPSE region, 0 for polygons */
static q3c_coord_t q3c_region_rad(q3c_region region, void *region_data)
{
    switch (region)
    {
    case Q3C_CIRCLE:
        return ((q3c_circle_region *)region_data)->rad;
    case Q3C_ELLIPSE:
        return ((q3c_ellipse_region *)region_data)->rad;
    default:
        return 0;
    }
}

/* Fill arr[pos..len) with empty [1,-1] ranges, as the SQL code wants
 * ipix arrays of fixed length */
static void q3c_fill_empty(q3c_ipix_t *arr, int pos, int len)
{
    while (pos < len)
    {
        arr[pos++] = 1;
        arr[pos++] = -1;
    }
}

/* Check that the point (alpha1, delta01) is within the ellipse with centre
 * (alpha, delta0), semi-major axis d0, eccentricity e and positional angle PA0
 * (east of north), all in degrees. The ellipse is defined in the gnomonic
 * (tangent-plane) projection around its centre: with u, v the tangent-plane
 * coordinates along the major and minor axes, the point is inside if
 * u^2/tan^2(d0) + v^2/(tan^2(d0) (1-e^2)) < 1 and it is less than 90 degrees
 * away from the centre. The inequality is multiplied by the positive
 * denominators, and the coordinates are computed without subtracting nearly
 * equal numbers, so that it stays accurate for tiny ellipses.
 */
char q3c_in_ellipse(q3c_coord_t alpha, q3c_coord_t delta0, q3c_coord_t alpha1, q3c_coord_t delta01, q3c_coord_t d0,
                    q3c_coord_t e, q3c_coord_t PA0)
{
    const q3c_coord_t d_alpha = (alpha1 - alpha) * Q3C_DEGRA, delta = delta0 * Q3C_DEGRA, delta1 = delta01 * Q3C_DEGRA,
                      PA = PA0 * Q3C_DEGRA, d = d0 * Q3C_DEGRA;
    const q3c_coord_t sin_delta = q3c_sin(delta), cos_delta = q3c_cos(delta), sin_delta1 = q3c_sin(delta1),
                      cos_delta1 = q3c_cos(delta1), sin_ddelta = q3c_sin((delta01 - delta0) * Q3C_DEGRA),
                      sin_half_dalpha = q3c_sin(d_alpha / 2), cos_half_dalpha = q3c_cos(d_alpha / 2),
                      sin_PA = q3c_sin(PA), cos_PA = q3c_cos(PA), sin_d = q3c_sin(d), cos_d = q3c_cos(d);
    /* (delta01 - delta0) is taken in degrees, where the subtraction of close values is exact */
    /* 1 - cos(d_alpha) */
    const q3c_coord_t vers_dalpha = 2 * sin_half_dalpha * sin_half_dalpha;
    /* cosine of the distance between the point and the centre */
    const q3c_coord_t cos_dist = sin_delta * sin_delta1 + cos_delta * cos_delta1 * (1 - vers_dalpha);
    /* tangent-plane coordinates (east, north) multiplied by cos_dist;
     * y is cos(delta) sin(delta1) - sin(delta) cos(delta1) cos(d_alpha) */
    const q3c_coord_t x = cos_delta1 * 2 * sin_half_dalpha * cos_half_dalpha;
    const q3c_coord_t y = sin_ddelta + sin_delta * cos_delta1 * vers_dalpha;
    /* the same along the major and minor axes */
    const q3c_coord_t u = x * sin_PA + y * cos_PA, v = -x * cos_PA + y * sin_PA;
    const q3c_coord_t e2 = 1 - e * e;

    if (cos_dist < 0)
    {
        return 0;
    }
    return (u * u * e2 + v * v) * cos_d * cos_d < sin_d * sin_d * e2 * cos_dist * cos_dist;
}

/* Index of the quadtree cell of level n0 (n0 cells per face side) containing
 * the face coordinate X in [-1, 1]. X == 1 (or (X + 1) / 2 rounding up to 1)
 * belongs to the last cell; without this clamp the index would be n0, which
 * is outside the face (and made the level search in q3c_get_nearby() spin).
 */
static q3c_ipix_t q3c_cell_index(q3c_coord_t X, q3c_ipix_t n0)
{
    q3c_ipix_t i = ((X + 1) / 2) * n0; /* C truncation of floats to integers */
    return i == n0 ? n0 - 1 : i;
}

/* The quadtree level n0 (a power of 2) whose cell size 2/n0 is the smallest
 * one >= the size of the box, but not smaller than a finest cell
 */
static q3c_ipix_t q3c_box_level(q3c_ipix_t nside, q3c_coord_t xmin, q3c_coord_t xmax, q3c_coord_t ymin,
                                q3c_coord_t ymax)
{
    q3c_coord_t esize = (xmax - xmin > ymax - ymin ? xmax - xmin : ymax - ymin);

    if (esize * nside < 2)
    /* If the region is too small */
    {
        esize = 2 / (q3c_coord_t)nside;
    }
    return 1 << ((q3c_ipix_t)(-q3c_ceil(q3c_log(esize / 2) / Q3C_LG2)));
}

/* Clamp the box (xmin,xmax,ymin,ymax) to the face, i.e. [-1,1] in
 * both coordinates. A degenerate (inverted) or NaN box is replaced by the
 * whole face, so that regions with degenerate projections still cover all
 * their pixels rather than losing them. Every bound is clamped from both
 * sides, so a box lying entirely outside the face becomes a zero-size box
 * on the face border rather than an inverted one.
 */
static void q3c_clip_facebox(q3c_coord_t *xmin, q3c_coord_t *xmax, q3c_coord_t *ymin, q3c_coord_t *ymax)
{
    if (!((*xmin <= *xmax) && (*ymin <= *ymax)))
    /* the check is written this way to catch NaNs as well */
    {
        *xmin = -1;
        *ymin = -1;
        *xmax = 1;
        *ymax = 1;
        return;
    }
    *xmax = (*xmax > 1 ? 1 : (*xmax < -1 ? -1 : *xmax));
    *xmin = (*xmin < -1 ? -1 : (*xmin > 1 ? 1 : *xmin));
    *ymax = (*ymax > 1 ? 1 : (*ymax < -1 ? -1 : *ymax));
    *ymin = (*ymin < -1 ? -1 : (*ymin > 1 ? 1 : *ymin));
}

/* Covers the (clipped) box on a face with at most nslots quadtree cells of
 * one level and writes their ipix ranges to *ipix_cur, advancing it.
 * It starts at the level whose cell is about the size of the box and goes
 * to coarser levels until the cells touched by the box fit in nslots (a box
 * can touch more cells than expected when its side lies exactly on a cell
 * boundary or when the level estimate is off by rounding); n0 == 1 is the
 * whole face, which always fits.
 */
static void q3c_push_box_cells(struct q3c_prm *hprm, char face_num, q3c_coord_t xmin, q3c_coord_t xmax,
                               q3c_coord_t ymin, q3c_coord_t ymax, int nslots, q3c_ipix_t **ipix_cur)
{
    const q3c_ipix_t nside = hprm->nside;
    q3c_ipix_t n0, n1, ix, iy, ixmin, ixmax, iymin, iymax, ipix0;

    for (n0 = q3c_box_level(nside, xmin, xmax, ymin, ymax);; n0 /= 2)
    {
        ixmin = q3c_cell_index(xmin, n0);
        ixmax = q3c_cell_index(xmax, n0);
        iymin = q3c_cell_index(ymin, n0);
        iymax = q3c_cell_index(ymax, n0);
        if ((ixmax - ixmin + 1) * (iymax - iymin + 1) <= nslots || n0 == 1)
        {
            break;
        }
    }
    n1 = nside / n0;
    for (ix = ixmin; ix <= ixmax; ix++)
    {
        for (iy = iymin; iy <= iymax; iy++)
        {
            ipix0 = q3c_xiyi2ipix(nside, hprm->xbits, hprm->ybits, face_num, ix * n1, iy * n1);
            *((*ipix_cur)++) = ipix0;
            *((*ipix_cur)++) = ipix0 + n1 * n1 - 1; /* -1 as the queries use <= ipix <= */
        }
    }
}

/* Returns 1 when the box does not overlap the face at all. Since the box
 * contains the projection of the region, this means the region does not
 * really touch this face and it can be skipped; this happens when a side
 * of the main-face box overflows because of the projection stretch rather
 * than because the region continues onto the neighbouring face.
 * (NaN-safe: returns 0 for NaN boxes.)
 */
static char q3c_facebox_disjoint(q3c_coord_t xmin, q3c_coord_t xmax, q3c_coord_t ymin, q3c_coord_t ymax)
{
    return (xmin > 1) || (xmax < -1) || (ymin > 1) || (ymax < -1);
}

/* Checking whether the box (xmin,ymin,xmax,ymax) intersects other faces or
 * not. If yes, I setup the array "points" designed to help us work on
 * other faces ( points array will then have the coordinates on a main face
 * which should be mapped to other faces
 * !!!!!!!!!!!!!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!!!! It does change the arguments (xmin,xmax, ymin, ymax) !!!!!!!!
 * !!!!!!!!!!!!!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * It only clamps the sides it detects as overflowing, so the callers must
 * clamp the resulting box themselves (see q3c_clip_facebox()).
 * A region bounded by Q3C_MAXRAD degrees in radius can really touch at
 * most three faces (the fourth-nearest face is always more than
 * Q3C_MAXRAD degrees away), but its *projection* on the main face can
 * spill beyond up to four sides of the face, because the gnomonic
 * projection stretches the parts of the region that are far from the
 * face. We cannot tell here which of the overflowing sides the region
 * really crosses on the sphere, so we report the neighbour behind every
 * overflowing side (up to four, so points[] must hold up to four points
 * and *multi_flag can be up to 4). The callers should drop the faces
 * which the region does not really touch with q3c_facebox_disjoint()
 * once they compute the box on each reported face; the callers that
 * cannot describe more than three extra faces must fall back to
 * searching the whole sky when *multi_flag == 4.
 */
void q3c_multi_face_check(q3c_coord_t *xmin0, q3c_coord_t *ymin0, q3c_coord_t *xmax0, q3c_coord_t *ymax0,
                          q3c_coord_t *points, char *multi_flag)
{
    const q3c_coord_t xmin = *xmin0, xmax = *xmax0, ymin = *ymin0, ymax = *ymax0;
    const char ovxmin = (xmin < -1), ovxmax = (xmax > 1), ovymin = (ymin < -1), ovymax = (ymax > 1);

    /* The probe points lie behind the middle of each overflowing side;
     * the box always intersects the face (the projection of the region
     * centre is inside both), so the clamped midpoints are within the
     * face and each probe maps to the neighbour behind its side.
     */
    const q3c_coord_t xc = ((ovxmin ? -1 : xmin) + (ovxmax ? 1 : xmax)) / 2;
    const q3c_coord_t yc = ((ovymin ? -1 : ymin) + (ovymax ? 1 : ymax)) / 2;
    int np = 0;

    if (ovxmin)
    {
        points[np++] = xmin;
        points[np++] = yc;
        *xmin0 = -1;
    }
    if (ovxmax)
    {
        points[np++] = xmax;
        points[np++] = yc;
        *xmax0 = 1;
    }
    if (ovymin)
    {
        points[np++] = xc;
        points[np++] = ymin;
        *ymin0 = -1;
    }
    if (ovymax)
    {
        points[np++] = xc;
        points[np++] = ymax;
        *ymax0 = 1;
    }
    *multi_flag = np / 2;
}

/* Get the list of 4 ipix ranges
 * which decsribe the neighborhood of a given point
 * specified by q3c_region
 * ra in degrees, dec in degrees, radius in degrees
 * strictly 0<=ra<360 and -90<=dec<=90
 * Returns 0 on success and 1 when the region touches more faces than the
 * four output ranges can describe; the latter should never happen and must
 * be reported by the caller as an internal error.
 */
int q3c_get_nearby(struct q3c_prm *hprm, q3c_region region, void *region_data, q3c_ipix_t *ipix)
{
    q3c_coord_t xmin, xmax, ymin, ymax, points[8];
    q3c_conic conic; /* not used here */
    const q3c_ipix_t nside = hprm->nside;
    q3c_ipix_t *ipix_cur = ipix;
    char face_num, face_num0, multi_flag, full_flag = 0;
    int i, nslots;

    if (q3c_region_rad(region, region_data) < 0)
    {
        q3c_fill_empty(ipix, 0, 8);
        return 0;
    }

    if (q3c_too_big_check(region, region_data))
    {
        /* the whole sky */
        ipix[0] = -1;
        ipix[1] = 6 * (nside * nside);
        q3c_fill_empty(ipix, 2, 8);
        return 0;
    }

    face_num = q3c_get_region_facenum(region, region_data);
    face_num0 = face_num;

    q3c_face_setup(face_num, region, region_data, &xmin, &xmax, &ymin, &ymax, &conic, &full_flag);
    /* xmin, xmax, ymin, ymax are face coordinates, the face is [-1,1]x[-1,1] */
#ifdef Q3C_DEBUG
    fprintf(stderr, "XMIN: %f XMAX: %f YMIN: %f YMAX: %f\n", xmin, xmax, ymin, ymax);
#endif

    q3c_multi_face_check(&xmin, &ymin, &xmax, &ymax, points, &multi_flag);
    /* q3c_multi_face_check() may leave some sides unclamped, so the
     * main-face box must be clamped here; otherwise a box wider than the
     * face gives a box size > 1 and a negative shift count in
     * q3c_push_box_cells()
     */
    q3c_clip_facebox(&xmin, &xmax, &ymin, &ymax);

    if (multi_flag > 3)
    {
        /* The main face and four neighbours do not fit in the four output
         * ranges. multi_flag == 4 requires the bounding box to spill over
         * all four sides of the main face at once, and the regions that
         * reach this point (circles/ellipses with rad <= Q3C_MAXRAD;
         * larger ones were sent to the whole-sky scan by
         * q3c_too_big_check() above) cannot do that: even the largest
         * allowed circle centred on the face only reaches
         * |x|,|y| = tan(Q3C_MAXRAD)/2 ~ 0.35, and any allowed ellipse is
         * contained in such a circle, so its box is smaller still
         * (verified numerically over all centres, radii and ellipse
         * shapes). Therefore this must never be reached and is reported
         * as an internal error.
         */
        return 1;
    }

    /* The four output ranges are shared between the main face and its
     * multi_flag neighbours: 4, 2+2, 1+1+1 or 1+1+1+1
     */
    nslots = (multi_flag == 0 ? 4 : (multi_flag == 1 ? 2 : 1));
    q3c_push_box_cells(hprm, face_num0, xmin, xmax, ymin, ymax, nslots, &ipix_cur);
    for (i = 0; i < multi_flag; i++)
    {
        face_num = q3c_xy2facenum(points[2 * i], points[2 * i + 1], face_num0);
        q3c_face_setup(face_num, region, region_data, &xmin, &xmax, &ymin, &ymax, &conic, &full_flag);
        /* skip a face the region does not really touch (only checked when
         * there are several neighbours; a single neighbour is always kept) */
        if (multi_flag > 1 && q3c_facebox_disjoint(xmin, xmax, ymin, ymax))
        {
            continue;
        }
        q3c_clip_facebox(&xmin, &xmax, &ymin, &ymax);
        q3c_push_box_cells(hprm, face_num, xmin, xmax, ymin, ymax, nslots, &ipix_cur);
    }

#ifdef Q3C_DEBUG
    fprintf(stderr, "MULTI_FLAG: %d\n", multi_flag);
#endif

    q3c_fill_empty(ipix, ipix_cur - ipix, 8);
    return 0;
}

/* Converts integer coordinates on cube face to
 * ipix number by performing bit interleaving
 */
q3c_ipix_t q3c_xiyi2ipix(const q3c_ipix_t nside, const q3c_ipix_t *xbits, const q3c_ipix_t *ybits, char face_num,
                         q3c_ipix_t xi, q3c_ipix_t yi)
{

    return ((q3c_ipix_t)face_num) * nside * nside + xbits[xi % Q3C_I1] + ybits[yi % Q3C_I1] +
           (xbits[(xi >> Q3C_INTERLEAVED_NBITS) % Q3C_I1] + ybits[(yi >> Q3C_INTERLEAVED_NBITS) % Q3C_I1]) * Q3C_I1 *
               Q3C_I1;
    /*8byte computation*/
}

/* ipix -> integer cell coordinates xi, yi; returns the face number.
 * Undoes q3c_xiyi2ipix() (COORDINATES.md section 5)
 */
static char q3c_ipix2xiyi(struct q3c_prm *hprm, q3c_ipix_t ipix, q3c_ipix_t *xi, q3c_ipix_t *yi)
{
    const q3c_ipix_t nside = hprm->nside, ii1 = 1 << (Q3C_INTERLEAVED_NBITS / 2);
    q3c_ipix_t i2 = ipix % (nside * nside), mult = 1;
    int k;

    /* every Q3C_INTERLEAVED_NBITS bits of ipix hold half as many bits of xi and yi */
    for (*xi = 0, *yi = 0, k = 0; k < 4; k++, i2 /= Q3C_I1, mult *= ii1)
    {
        *xi += hprm->xbits1[i2 % Q3C_I1] * mult;
        *yi += hprm->ybits1[i2 % Q3C_I1] * mult;
    }
    return ipix / (nside * nside);
}

/* (face, X, Y) -> (ra, dec) in degrees, X, Y in [-1, 1] (COORDINATES.md section 3) */
static void q3c_XY2ang(char face_num, q3c_coord_t X, q3c_coord_t Y, q3c_coord_t *ra, q3c_coord_t *dec)
{
    q3c_coord_t ra0;

    if ((face_num >= 1) && (face_num <= 4))
    {
        ra0 = q3c_atan(X);
        *dec = Q3C_RADEG * q3c_atan(Y * q3c_cos(ra0));
        ra0 = ra0 * Q3C_RADEG + ((q3c_coord_t)face_num - 1) * 90;
        if (ra0 < 0)
        {
            ra0 += (q3c_coord_t)360;
        }
        *ra = ra0;
    }
    else if (face_num == 0)
    {
        *ra = Q3C_RADEG * (q3c_atan2(-X, Y) + Q3C_PI);
        *dec = Q3C_RADEG * q3c_atan(1 / q3c_sqrt(X * X + Y * Y));
    }
    else if (face_num == 5)
    {
        *ra = Q3C_RADEG * (q3c_atan2(-X, -Y) + Q3C_PI);
        *dec = -Q3C_RADEG * q3c_atan(1 / q3c_sqrt(X * X + Y * Y));
    }
}

/* convert ipix number to ra,dec in degrees of the cell centre */
void q3c_ipix2ang(struct q3c_prm *hprm, q3c_ipix_t ipix, q3c_coord_t *ra, q3c_coord_t *dec)
{
    const q3c_ipix_t nside = hprm->nside;
    q3c_ipix_t xi, yi;
    char face_num = q3c_ipix2xiyi(hprm, ipix, &xi, &yi);

    q3c_XY2ang(face_num, (((q3c_coord_t)xi + .5) / nside) * 2 - 1, (((q3c_coord_t)yi + .5) / nside) * 2 - 1, ra,
               dec);
}

/* Compute the area of a given Q3C pixel for a given ipix and depth
 * depth here goes from 0 to 30 in the case of 8byte ints
 * and means depth == 0 pixel is the the smallest pixel, and depth== 30 pixel
 * is the whole cube face
 */
q3c_coord_t q3c_pixarea(struct q3c_prm *hprm, q3c_ipix_t ipix, int depth)
{
    const q3c_ipix_t nside = hprm->nside;
    q3c_ipix_t xi, yi, idx, ix1, iy1, ix2, iy2;
    q3c_coord_t x1, y1, x2, y2, result;

    q3c_ipix2xiyi(hprm, ipix, &xi, &yi);
    ix1 = (xi >> depth) << depth;
    iy1 = (yi >> depth) << depth;
    idx = ((q3c_ipix_t)1) << depth;
    ix2 = ix1 + idx;
    iy2 = iy1 + idx;

    /* the cell corners in X, Y */
    x1 = (((q3c_coord_t)ix1) / nside) * 2 - 1;
    y1 = (((q3c_coord_t)iy1) / nside) * 2 - 1;
    x2 = (((q3c_coord_t)ix2) / nside) * 2 - 1;
    y2 = (((q3c_coord_t)iy2) / nside) * 2 - 1;

    x1 = x1 / q3c_sqrt(1 + x1 * x1);
    y1 = y1 / q3c_sqrt(1 + y1 * y1);
    x2 = x2 / q3c_sqrt(1 + x2 * x2);
    y2 = y2 / q3c_sqrt(1 + y2 * y2);

    if (q3c_fabs(x1 - x2) > 1e-4)
    {
        result = (q3c_acos(x1 * y2) - q3c_acos(x1 * y1)) + (q3c_acos(x2 * y1) - q3c_acos(x2 * y2));
    }
    else
    {
        result = q3c_asin(x1 * (y1 - y2) * (y1 + y2) /
                          (y2 * sqrt(1 - x1 * x1 * y1 * y1) + y1 * sqrt(1 - x1 * x1 * y2 * y2))) +
                 q3c_asin(x2 * (y2 - y1) * (y1 + y2) /
                          (y1 * sqrt(1 - x2 * x2 * y2 * y2) + y2 * sqrt(1 - x2 * x2 * y1 * y1)));
    }
    result = q3c_fabs(result);
    return result;
}

/* Find the face which contains the point with face coordinates (X, Y)
 * relative to face face_num0; X, Y may lie outside [-1, 1]
 */
char q3c_xy2facenum(q3c_coord_t X, q3c_coord_t Y, char face_num0)
{
    q3c_coord_t ra = 0, dec = 0;

    q3c_XY2ang(face_num0, X, Y, &ra, &dec);
    return q3c_get_facenum(ra, dec);
}

/* Initialization of the Q3CUBE structure
 * hprm -- Pointer to main Q3C structure
 * nside -- Nside parameter (number of quadtree subdivisions)
 */
void init_q3c1(struct q3c_prm_write *hprm, q3c_ipix_t nside)
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
    for (i = 2, m = 1; i < xybits_size; i++)
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

    for (i = 2, m = 2, l = 2; i < xybits_size; i++)
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

    ybits1[0] = 0;
    ybits1[1] = 0;
    /*
       fprintf(stdout,"%lld\n",ybits1[0]);
       fprintf(stdout,"%lld\n",ybits1[1]);
     */

    for (i = 2, m = 1, l = 1; i < xybits_size; i++)
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
void q3c_dump_prm(struct q3c_prm_write *hprm, char *filename)
{
    FILE *fp = fopen(filename, "w");
    int i, j, x = 1 << Q3C_INTERLEAVED_NBITS;
    const char *names[4] = {"xbits", "ybits", "xbits1", "ybits1"};
    const q3c_ipix_t *arrs[4] = {hprm->xbits, hprm->ybits, hprm->xbits1, hprm->ybits1};

    fprintf(fp, "#include \"common.h\"\n");
    for (j = 0; j < 4; j++)
    {
        fprintf(fp, "\nconst q3c_ipix_t ____%s[%d]={ ", names[j], x);
        for (i = 0; i < x; i++)
        {
            fprintf(fp, "%sQ3C_CONST(" Q3C_IPIX_FMT ")", i > 0 ? "," : "", arrs[j][i]);
        }
        fprintf(fp, "};");
    }
    fprintf(fp, "\n");

    fprintf(fp,"const struct q3c_prm hprm={" Q3C_IPIX_FMT ",____xbits,____ybits,____xbits1,____ybits1};\n",
            hprm->nside);
    fclose(fp);
}

/* Check if the region is too big for Q3C to handle properly
 * The limit here is related to the number of faces the region is
 * allowed to intersect. Q3C can't handle more than 3 faces.
 * So for such big regions, we just switch to scan of the whole table
 */

char q3c_too_big_check(q3c_region region, void *region_data)
{
    return q3c_region_rad(region, region_data) > Q3C_MAXRAD;
}

/* Components t = (v.n_f, v.u_f, v.w_f) of the vector v in the frame of face
 * face_num (COORDINATES.md section 2), a signed permutation of v
 */
static void q3c_to_face_frame(char face_num, const q3c_coord_t v[3], q3c_coord_t t[3])
{
    static const char idx[6][3] = {{2, 1, 0}, {0, 1, 2}, {1, 0, 2}, {0, 1, 2}, {1, 0, 2}, {2, 1, 0}};
    static const signed char sgn[6][3] = {{1, 1, -1}, {1, 1, 1}, {1, -1, 1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, 1}};
    int k;

    for (k = 0; k < 3; k++)
    {
        t[k] = sgn[(int)face_num][k] * v[(int)idx[(int)face_num][k]];
    }
}

/* Bounding box (xmin, xmax, ymin, ymax) and the conic cn on face face_num of
 * the ellipse with centre (ra0, dec0), semi-major axis rad, eccentricity e and
 * positional angle PA (all in degrees), defined as in q3c_in_ellipse().
 * With c the centre, m and n the unit vectors along the major and minor axes
 * at c, s = sin(rad), k = cos(rad) and e2 = 1 - e^2, the point v is inside if
 *     k^2 (e2 (v.m)^2 + (v.n)^2) - e2 s^2 (v.c)^2 < 0,
 * i.e. p^T A p < 0 for p = (1, X, Y) and c, m, n taken in the face frame
 * (COORDINATES.md section 6). The tangent lines X = t of the curve satisfy
 * l^T B l = 0 for l = (-t, 1, 0), where B ~ A^-1 is
 *     B = s^2 (m m^T + e2 n n^T) - k^2 c c^T;
 * the discriminants of these quadratic equations have s^2 as an exact
 * factor, so the box stays accurate for tiny ellipses.
 * When the curve on the face is not an ellipse (B00 >= 0) *full_flag is set
 * and the box is [-2,2]x[-2,2], i.e. larger than the face.
 */
static void q3c_ellipse_face(char face_num, q3c_coord_t ra0, q3c_coord_t dec0, q3c_coord_t rad, q3c_coord_t e,
                             q3c_coord_t PA, q3c_coord_t *xmin, q3c_coord_t *xmax, q3c_coord_t *ymin,
                             q3c_coord_t *ymax, q3c_conic *cn, char *full_flag)
{
    const q3c_coord_t e2 = 1 - e * e;
    q3c_coord_t sa, ca, sd, cd, sp, cp, s, k, v[3], c[3], m[3], n[3], P, Dx, Dy;

    q3c_sincos(ra0 * Q3C_DEGRA, sa, ca);
    q3c_sincos(dec0 * Q3C_DEGRA, sd, cd);
    q3c_sincos(PA * Q3C_DEGRA, sp, cp);
    q3c_sincos(rad * Q3C_DEGRA, s, k);
    /* the centre, m = sin(PA) east + cos(PA) north, n = -cos(PA) east + sin(PA) north */
    v[0] = cd * ca, v[1] = cd * sa, v[2] = sd;
    q3c_to_face_frame(face_num, v, c);
    v[0] = -sp * sa - cp * sd * ca, v[1] = sp * ca - cp * sd * sa, v[2] = cp * cd;
    q3c_to_face_frame(face_num, v, m);
    v[0] = cp * sa - sp * sd * ca, v[1] = -cp * ca - sp * sd * sa, v[2] = sp * cd;
    q3c_to_face_frame(face_num, v, n);

#define A(i, j) (k * k * (e2 * m[i] * m[j] + n[i] * n[j]) - e2 * s * s * c[i] * c[j])
#define S(i, j) (m[i] * m[j] + e2 * n[i] * n[j])
    P = k * k * c[0] * c[0] - s * s * S(0, 0); /* -B00 */
    /* (B01^2 - B00 B11) / s^2 and (B02^2 - B00 B22) / s^2 */
    Dx = k * k * (c[0] * c[0] * S(1, 1) + c[1] * c[1] * S(0, 0) - 2 * c[0] * c[1] * S(0, 1)) -
         s * s * (S(0, 0) * S(1, 1) - S(0, 1) * S(0, 1));
    Dy = k * k * (c[0] * c[0] * S(2, 2) + c[2] * c[2] * S(0, 0) - 2 * c[0] * c[2] * S(0, 2)) -
         s * s * (S(0, 0) * S(2, 2) - S(0, 2) * S(0, 2));
    if (!(P >= Q3C_MINDISCR) || !(Dx >= 0) || !(Dy >= 0))
    /* written this way to catch NaNs as well */
    {
        *xmin = *ymin = -2;
        *xmax = *ymax = 2;
        *full_flag = 1;
        return;
    }
    /* The centre projects to p_c = c / c0 = (1, X0, Y0) and A c = -e2 s^2 c,
     * so with p = p_c + (0, dX, dY) and d.c = c1 dX + c2 dY
     *     p^T A p = dX, dY quadratic part - 2 e2 s^2 (d.c) / c0 - e2 s^2 / c0^2
     */
    cn->x0 = c[1] / c[0];
    cn->y0 = c[2] / c[0];
    cn->axx = A(1, 1);
    cn->ayy = A(2, 2);
    cn->axy = 2 * A(1, 2);
    cn->ax = -2 * e2 * s * s * c[1] / c[0];
    cn->ay = -2 * e2 * s * s * c[2] / c[0];
    cn->a = -e2 * s * s / (c[0] * c[0]);
    /* the box edges X = (-B01 -+ s sqrt(Dx)) / -B00, written relative to X0 */
    *xmin = cn->x0 + (s * s * (c[1] * S(0, 0) - c[0] * S(0, 1)) / c[0] - s * q3c_sqrt(Dx)) / P;
    *xmax = cn->x0 + (s * s * (c[1] * S(0, 0) - c[0] * S(0, 1)) / c[0] + s * q3c_sqrt(Dx)) / P;
    *ymin = cn->y0 + (s * s * (c[2] * S(0, 0) - c[0] * S(0, 2)) / c[0] - s * q3c_sqrt(Dy)) / P;
    *ymax = cn->y0 + (s * s * (c[2] * S(0, 0) - c[0] * S(0, 2)) / c[0] + s * q3c_sqrt(Dy)) / P;
#undef A
#undef S
}

/* Bounding box and conic on face face_num of the circle with centre (ra0, dec0)
 * and radius rad (degrees). This is q3c_ellipse_face() for e = 0, where
 * m m^T + n n^T = I - c c^T, so A = k^2 I - c c^T and B = s^2 I - c c^T.
 */
static void q3c_circle_face(char face_num, q3c_coord_t ra0, q3c_coord_t dec0, q3c_coord_t rad, q3c_coord_t *xmin,
                            q3c_coord_t *xmax, q3c_coord_t *ymin, q3c_coord_t *ymax, q3c_conic *cn, char *full_flag)
{
    q3c_coord_t sa, ca, sd, cd, s, k, v[3], c[3], P, Dx, Dy;

    q3c_sincos(ra0 * Q3C_DEGRA, sa, ca);
    q3c_sincos(dec0 * Q3C_DEGRA, sd, cd);
    q3c_sincos(rad * Q3C_DEGRA, s, k);
    v[0] = cd * ca, v[1] = cd * sa, v[2] = sd;
    q3c_to_face_frame(face_num, v, c);

    P = c[0] * c[0] - s * s;            /* -B00 */
    Dx = c[0] * c[0] + c[1] * c[1] - s * s; /* (B01^2 - B00 B11) / s^2 */
    Dy = c[0] * c[0] + c[2] * c[2] - s * s;
    if (!(P >= Q3C_MINDISCR) || !(Dx >= 0) || !(Dy >= 0))
    /* not an ellipse on this face (also catches NaNs) */
    {
        *xmin = *ymin = -2;
        *xmax = *ymax = 2;
        *full_flag = 1;
        return;
    }
    /* relative to the centre, as in q3c_ellipse_face() with e = 0 */
    cn->x0 = c[1] / c[0];
    cn->y0 = c[2] / c[0];
    cn->axx = k * k - c[1] * c[1];
    cn->ayy = k * k - c[2] * c[2];
    cn->axy = -2 * c[1] * c[2];
    cn->ax = -2 * s * s * c[1] / c[0];
    cn->ay = -2 * s * s * c[2] / c[0];
    cn->a = -s * s / (c[0] * c[0]);
    *xmin = cn->x0 + (s * s * c[1] / c[0] - s * q3c_sqrt(Dx)) / P;
    *xmax = cn->x0 + (s * s * c[1] / c[0] + s * q3c_sqrt(Dx)) / P;
    *ymin = cn->y0 + (s * s * c[2] / c[0] - s * q3c_sqrt(Dy)) / P;
    *ymax = cn->y0 + (s * s * c[2] / c[0] + s * q3c_sqrt(Dy)) / P;
}

/* Function checking whether the square with center xc_cur, yc_cur and the
 * size cur_size on the cube face lie inside, or intersects etc. with the
 * circle/ellipse with the bounding box (xmin, xmax, ymin, ymax) specified
 * by the conic c
 */
static char q3c_conic_cover_check(q3c_coord_t xc_cur, q3c_coord_t yc_cur, q3c_coord_t cur_size, q3c_coord_t xmin,
                                  q3c_coord_t xmax, q3c_coord_t ymin, q3c_coord_t ymax, const q3c_conic *c)
{
    const q3c_coord_t axx = c->axx, ayy = c->ayy, axy = c->axy, ax = c->ax, ay = c->ay, a = c->a;
    q3c_coord_t xl_cur, xr_cur, yb_cur, yt_cur, xl, xr, yb, yt;
    char status = 0, inside;

    /* Checking the intersection of ellipse and box
     * The box parameters are set by variables xc_cur, yc_cur and cur_size
     */
    xl_cur = xc_cur - cur_size / 2; /* left   */
    xr_cur = xc_cur + cur_size / 2; /* right  */
    yb_cur = yc_cur - cur_size / 2; /* bottom */
    yt_cur = yc_cur + cur_size / 2; /* top    */
    /* the same relative to the region centre, where the curve is defined */
    xl = xl_cur - c->x0;
    xr = xr_cur - c->x0;
    yb = yb_cur - c->y0;
    yt = yt_cur - c->y0;

#define EVAL_POLY(x, y) (x * (axx * x + axy * y + ax) + y * (ayy * y + ay) + a)

    /* the idea of the code is following:
     * we go through the list of vertices till we encounter the one
     * which is inside the ellipse
     * If none of the vertices is inside the ellipse we double check the
     * edge crossings
     * If all the vertices are inside we return "covered"
     */

    /* Bottom left vertex */
    inside = EVAL_POLY(xl, yb) < 0;
    status += inside;

    /* Bottom right vertex */
    inside = EVAL_POLY(xr, yb) < 0;
    /* we use XOR to check if the current vertex has different status
     * than the previous ones */
    if (inside ^ (status > 0))
    {
        return Q3C_PARTIAL;
    }
    status += inside;

    /* Top right vertex */
    inside = EVAL_POLY(xr, yt) < 0;
    if (inside ^ (status > 0))
    {
        return Q3C_PARTIAL;
    }
    status += inside;

    /* Top left vertex */
    inside = EVAL_POLY(xl, yt) < 0;
    if (inside ^ (status > 0))
    {
        return Q3C_PARTIAL;
    }
    status += inside;
#undef EVAL_POLY

    if (status == 4)
    {
        /* All the vertices are inside so the square must be covered by
         * the ellipse */
        return Q3C_COVER;
    }
    /* All the vertices are outside the ellipse
     * Now we test if the ellipse crosses the edges of the square
     */
    if ((Q3C_INTERSECT(xmin, xmax, xl_cur, xr_cur) && Q3C_INTERSECT(ymin, ymax, yb_cur, yt_cur)) &&
        ((((2 * axx * xl + axy * yt + ax) * (2 * axx * xr + axy * yt + ax)) < 0) ||
         (((2 * axx * xl + axy * yb + ax) * (2 * axx * xr + axy * yb + ax)) < 0) ||
         (((2 * ayy * yb + axy * xl + ay) * (2 * ayy * yt + axy * xl + ay)) < 0) ||
         (((2 * ayy * yb + axy * xr + ay) * (2 * ayy * yt + axy * xr + ay)) < 0)))
    {
        return Q3C_PARTIAL;
    }
    return Q3C_DISJUNCT;
}

/* Try to describe the box on the cube face specified by (xmin,xmax,ymin,ymax)
 * as a set of quadtree squares of level n0 (the box is not larger than a
 * square, so it touches at most 2x2 of them) and put them in the stack.
 * Returns the number of squares.
 */
static int q3c_setup_square_stack(struct q3c_square *stack, q3c_coord_t xmin, q3c_coord_t ymin, q3c_coord_t xmax,
                                  q3c_coord_t ymax, int n0)
{
    const q3c_ipix_t ix[2] = {q3c_cell_index(xmin, n0), q3c_cell_index(xmax, n0)},
                     iy[2] = {q3c_cell_index(ymin, n0), q3c_cell_index(ymax, n0)};
    int i, j, nstack = 0;

    for (i = 0; i <= (ix[1] != ix[0]); i++)
    {
        for (j = 0; j <= (iy[1] != iy[0]); j++, nstack++)
        {
            SET_SQUARE((&stack[nstack]), ix[i], iy[j], n0);
        }
    }
    return nstack;
}

/* Number of refinement passes for the quadtree starting at n0: only the
 * first res_depth-1 passes split the squares, so n0 * 2^(res_depth-1) must
 * not exceed nside, i.e. res_depth = 1 + log2(nside / n0), capped at
 * max_depth. (nside / n0 is always a power of two.)
 */
static int q3c_res_depth(q3c_ipix_t nside, q3c_ipix_t n0, int max_depth)
{
    int d;
    for (d = 1; d < max_depth && (n0 << d) <= nside; d++)
        ;
    return d;
}

/* Process the stack of quad-tree squares and depending on the overlap status
 * I either keep them, expand them into smaller squares or throw them out
 */
static void q3c_stack_expand(struct q3c_square *work_stack, int *work_nstack, struct q3c_square *out_stack, int *out_nstack,
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
    q3c_ipix_t xtmp, ytmp, ntmp;
    int j, k;

    for (j = 0, tmp_stack1 = 0, tmp_stack2 = *work_nstack; j < *work_nstack; j++)
    {
        cur_square = work_stack + j;
        // fprintf(stdout,"%d %d %d\n",work_nstack,tmp_stack1,tmp_stack2);
        if (cur_square->status == Q3C_PARTIAL)
        /* If this square partially intersects with the ellipse
         * I should split this square further
         */
        {

            /* If this is the last stage of resolution loop, I will not split
             * the "partial" boxes
             */
            if (cur_depth == res_depth)
                continue;
            tmp_stack1++;

            xtmp = 2 * cur_square->ix;
            ytmp = 2 * cur_square->iy;
            ntmp = 2 * cur_square->n0;

            /* First I try to put the childrens of this square in the part of
             * the stack freed by thrown away squares (which were disjunct from
             * the ellipse or which were fully covered by the ellipse)
             */
            for (k = 0; (k <= 3) && (tmp_stack1 > 0); k++)
            {
                cur_square = work_stack + (j + 1 - tmp_stack1);
                SET_SQUARE(cur_square, xtmp + (k & 1), ytmp + ((k & 2) >> 1), ntmp);
                tmp_stack1--;
            }

            for (; k <= 3; k++)
            {
                cur_square = work_stack + tmp_stack2;
                SET_SQUARE(cur_square, xtmp + (k & 1), ytmp + ((k & 2) >> 1), ntmp);
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

    if (cur_depth == res_depth)
        return;
    /* After updating the list of squares I compute how many of the
     * I have them now (except for the case of last resolution step)
     */
#ifdef Q3C_DEBUG
    fprintf(stdout, "STACK STATE nw_stack: %d nt_stack1: %d nt_stack2: %d\n", *work_nstack, tmp_stack1, tmp_stack2);
#endif

    if (tmp_stack1 == 0)
    {
        *work_nstack = tmp_stack2;
    }
    else
    {
        if ((tmp_stack2 - *work_nstack) > tmp_stack1)
        {
            memcpy(work_stack + (*work_nstack - tmp_stack1), work_stack + (tmp_stack2 - tmp_stack1),
                   tmp_stack1 * sizeof(struct q3c_square));
            *work_nstack = tmp_stack2 - tmp_stack1;
        }
        else
        {
            memcpy(work_stack + (*work_nstack - tmp_stack1), work_stack + *work_nstack,
                   (tmp_stack2 - *work_nstack) * sizeof(struct q3c_square));
            *work_nstack = tmp_stack2 - tmp_stack1;
        }
    }
}

/* Append the ipix range [ipix_start, ipix_end) of the quadtree square sq on
 * face face_num to arr (maxlen values long). Returns 1 if it does not fit.
 */
static int q3c_push_square(struct q3c_prm *hprm, const struct q3c_square *sq, char face_num, q3c_ipix_t *arr,
                           int *pos, int maxlen)
{
    const q3c_ipix_t nside = hprm->nside, m = nside / sq->n0; /* finest cells per square side */
    const q3c_ipix_t ipix0 = q3c_xiyi2ipix(nside, hprm->xbits, hprm->ybits, face_num, sq->ix * m, sq->iy * m);

    if ((*pos + 2) > maxlen)
    {
        return 1;
    }
    arr[(*pos)++] = ipix0;
    arr[(*pos)++] = ipix0 + m * m;
    return 0;
}

/* This function puts the fully covered squares (out_stack) and the partially
 * covered squares (those in work_stack with status Q3C_PARTIAL) in the
 * appropriate output lists.
 * It returns 0 on success and 1 if the ranges did not fit in the output
 * arrays, which hold Q3C_NFULLS/Q3C_NPARTIALS pairs each. In the latter case
 * q3c_region_query() redoes the decomposition with a smaller res_depth,
 * which produces fewer and coarser ranges.
 */
static int q3c_output_stack(struct q3c_prm *hprm, struct q3c_square *out_stack, int out_nstack,
                            struct q3c_square *work_stack, int work_nstack, char face_num,
                            q3c_ipix_t *out_ipix_arr_fulls, int *out_ipix_arr_fulls_pos,
                            q3c_ipix_t *out_ipix_arr_partials, int *out_ipix_arr_partials_pos)
{
    int i;

    for (i = 0; i < out_nstack; i++)
    {
        if (q3c_push_square(hprm, out_stack + i, face_num, out_ipix_arr_fulls, out_ipix_arr_fulls_pos,
                            2 * Q3C_NFULLS))
        {
            return 1;
        }
    }
    for (i = 0; i < work_nstack; i++)
    {
        if ((work_stack[i].status == Q3C_PARTIAL) &&
            q3c_push_square(hprm, work_stack + i, face_num, out_ipix_arr_partials, out_ipix_arr_partials_pos,
                            2 * Q3C_NPARTIALS))
        {
            return 1;
        }
    }
    return 0;
}

/* Compute the bounding box (xmin, xmax, ymin, ymax) of the region on face
 * face_num and what the cover checks need on that face: the conic c for
 * circles/ellipses (*full_flag is set when the curve on the face is not an
 * ellipse, then the box is [-1,1]x[-1,1] and the whole face is taken) or the
 * polygon projected on the face (q3c_get_nearby() is never called for those).
 * Returns 1 when the polygon is too large to be projected on the face.
 */
static char q3c_face_setup(char face_num, q3c_region region, void *region_data, q3c_coord_t *xmin, q3c_coord_t *xmax,
                           q3c_coord_t *ymin, q3c_coord_t *ymax, q3c_conic *c, char *full_flag)
{
    switch (region)
    {
    case Q3C_CIRCLE: {
        const q3c_circle_region *r = region_data;
        q3c_circle_face(face_num, r->ra, r->dec, r->rad, xmin, xmax, ymin, ymax, c, full_flag);
        return 0;
    }
    case Q3C_ELLIPSE: {
        const q3c_ellipse_region *r = region_data;
        q3c_ellipse_face(face_num, r->ra, r->dec, r->rad, r->e, r->PA, xmin, xmax, ymin, ymax, c, full_flag);
        return 0;
    }
    default: {
        char large_flag = 0;
        q3c_project_poly(region_data, face_num, &large_flag);
        if (large_flag)
        {
            return 1;
        }
        q3c_prepare_poly(region_data);
        q3c_get_minmax_poly(region_data, xmin, xmax, ymin, ymax);
        return 0;
    }
    }
}

/* Quadtree decomposition of the region into fully and partially covered ipix
 * ranges using at most max_depth refinement passes.
 * Returns 0 on success and 1 when the produced ranges could not fit in the
 * output arrays.
 * A region spanning 3-4 cube faces can produce more ranges than the output
 * arrays hold. q3c_region_query() then retries with fewer refinement
 * passes, i.e. fewer and coarser ranges. With a single pass every face
 * contributes at most 4 squares (or one whole-face range), so at most 5 faces
 * give <= 20 pairs, which always fits; its final return 1 is only a guard.
 */
static int q3c_region_query_depth(struct q3c_prm *hprm, q3c_region region, void *region_data,
                                  q3c_ipix_t *out_ipix_arr_fulls, q3c_ipix_t *out_ipix_arr_partials, char *too_large,
                                  int max_depth)
{
    q3c_coord_t xmin, xmax, ymin, ymax, xc_cur, yc_cur, cur_size, points[8];
    q3c_conic conic;
    const q3c_ipix_t nside = hprm->nside;
    q3c_ipix_t n0;
    char face_num, face_num0, multi_flag = 0, face_count, full_flag = 0;
    int out_ipix_arr_fulls_pos = 0, out_ipix_arr_partials_pos = 0;
    int work_nstack, out_nstack, i, j, res_depth;
    struct q3c_square work_stack[Q3C_STACK_SIZE], out_stack[Q3C_STACK_SIZE], *cur_square;

    face_num0 = face_num = q3c_get_region_facenum(region, region_data);

    if (q3c_face_setup(face_num, region, region_data, &xmin, &xmax, &ymin, &ymax, &conic, &full_flag))
    {
        *too_large = 1;
        return 0;
    }

    q3c_multi_face_check(&xmin, &ymin, &xmax, &ymax, points, &multi_flag);

    for (face_count = 0; face_count <= multi_flag; face_count++)
    {
        /* This the beginning of the mega-loop over multiple faces */
        if (face_count > 0)
        /* This "if" works when we pass through the secondary faces */
        {
            face_num = q3c_xy2facenum(points[2 * face_count - 2], points[2 * face_count - 1], face_num0);
            full_flag = 0;
            if (q3c_face_setup(face_num, region, region_data, &xmin, &xmax, &ymin, &ymax, &conic, &full_flag))
            {
                *too_large = 1;
                return 0;
            }
            if (q3c_facebox_disjoint(xmin, xmax, ymin, ymax))
            /* the region does not really touch this face */
            {
                continue;
            }
        }
        /* clamp the box of every face, including the main one */
        q3c_clip_facebox(&xmin, &xmax, &ymin, &ymax);

#ifdef Q3C_DEBUG
        fprintf(stdout,
                "FACE RUN: %d FACE_NUM: %d FULL_FLAG: %d XMIN: " Q3C_COORD_FMT " XMAX: " Q3C_COORD_FMT
                " YMIN: " Q3C_COORD_FMT " YMAX: " Q3C_COORD_FMT "\n",
                face_count, face_num, full_flag, xmin, xmax, ymin, ymax);
#endif

        if (full_flag)
        /* Take the whole face */
        {
            if ((out_ipix_arr_partials_pos + 2) > (2 * Q3C_NPARTIALS))
            {
                return 1;
            }
            out_ipix_arr_partials[out_ipix_arr_partials_pos++] = face_num * nside * nside;
            out_ipix_arr_partials[out_ipix_arr_partials_pos++] = (face_num + 1) * nside * nside;
            continue;
        }

        /* the squares of level n0 are about the size of the region */
        n0 = q3c_box_level(nside, xmin, xmax, ymin, ymax);
        work_nstack = q3c_setup_square_stack(work_stack, xmin, ymin, xmax, ymax, n0);
        out_nstack = 0;
        res_depth = q3c_res_depth(nside, n0, max_depth);

        for (i = 1; i <= res_depth; i++)
        {
            /* This loop perform the testing of all squares in work_stack */
            for (j = 0; j < work_nstack; j++)
            {
                cur_square = work_stack + j;
                cur_size = ((q3c_coord_t)2) / (cur_square->n0);
                /* xc_cur and yc_cur -- center of the square in the face coordinates */
                xc_cur = (2 * cur_square->ix + 1) / (q3c_coord_t)cur_square->n0 - 1;
                yc_cur = (2 * cur_square->iy + 1) / (q3c_coord_t)cur_square->n0 - 1;
                cur_square->status =
                    (region == Q3C_POLYGON)
                        ? q3c_poly_cover_check(region_data, xc_cur, yc_cur, cur_size)
                        : q3c_conic_cover_check(xc_cur, yc_cur, cur_size, xmin, xmax, ymin, ymax, &conic);
            }
            q3c_stack_expand(work_stack, &work_nstack, out_stack, &out_nstack, i, res_depth);
        }

        if (q3c_output_stack(hprm, out_stack, out_nstack, work_stack, work_nstack, face_num, out_ipix_arr_fulls,
                             &out_ipix_arr_fulls_pos, out_ipix_arr_partials, &out_ipix_arr_partials_pos))
        {
            return 1;
        }

    } /* End of the mega-loop over the faces */

    q3c_fill_empty(out_ipix_arr_fulls, out_ipix_arr_fulls_pos, 2 * Q3C_NFULLS);
    q3c_fill_empty(out_ipix_arr_partials, out_ipix_arr_partials_pos, 2 * Q3C_NPARTIALS);
    return 0;
}

/* Main query function for all the region types.
 * Returns 0 on success and 1 when the produced ranges could not fit in the
 * output arrays; the latter should never happen and must be reported by the
 * caller as an internal error.
 */
static int q3c_region_query(struct q3c_prm *hprm, q3c_region region, void *region_data,
                            q3c_ipix_t *out_ipix_arr_fulls, q3c_ipix_t *out_ipix_arr_partials, char *too_large)
{
    const q3c_coord_t rad = q3c_region_rad(region, region_data);
    int max_depth;

    if (rad < 0)
    {
        q3c_fill_empty(out_ipix_arr_fulls, 0, 2 * Q3C_NFULLS);
        q3c_fill_empty(out_ipix_arr_partials, 0, 2 * Q3C_NPARTIALS);
        return 0;
    }

    /* 35 degrees is a magic size above which the cone from the search can
     * produce a hyperbola or a parabola on a main face and where a lot of
     * code will start to break.
     * So if the query is that large, I just query the whole sphere
     * (everything is partially covered)
     */
    if (rad >= Q3C_MAXRAD)
    {
        out_ipix_arr_partials[0] = -1;
        out_ipix_arr_partials[1] = 6 * (hprm->nside * hprm->nside);
        q3c_fill_empty(out_ipix_arr_fulls, 0, 2 * Q3C_NFULLS);
        q3c_fill_empty(out_ipix_arr_partials, 2, 2 * Q3C_NPARTIALS);
        return 0;
    }

    for (max_depth = Q3C_MAX_DEPTH; max_depth >= 1; max_depth--)
    {
        if (!q3c_region_query_depth(hprm, region, region_data, out_ipix_arr_fulls, out_ipix_arr_partials, too_large,
                                    max_depth))
        {
            return 0;
        }
    }
    return 1;
}

int q3c_radial_query(struct q3c_prm *hprm, q3c_coord_t ra0, q3c_coord_t dec0, q3c_coord_t rad,
                     q3c_ipix_t *out_ipix_arr_fulls, q3c_ipix_t *out_ipix_arr_partials)
{
    q3c_circle_region circle = {ra0, dec0, rad};
    char too_large = 0;
    return q3c_region_query(hprm, Q3C_CIRCLE, &circle, out_ipix_arr_fulls, out_ipix_arr_partials, &too_large);
}

int q3c_ellipse_query(struct q3c_prm *hprm, q3c_coord_t ra0, q3c_coord_t dec0, q3c_coord_t majax, q3c_coord_t ell,
                      q3c_coord_t PA, q3c_ipix_t *out_ipix_arr_fulls, q3c_ipix_t *out_ipix_arr_partials)
{
    q3c_ellipse_region ellipse = {ra0, dec0, majax, ell, PA};
    char too_large = 0;
    return q3c_region_query(hprm, Q3C_ELLIPSE, &ellipse, out_ipix_arr_fulls, out_ipix_arr_partials, &too_large);
}

/* *too_large is set when the polygon is too large to be projected on the
 * cube faces, the output arrays are then not filled */
int q3c_poly_query(struct q3c_prm *hprm, q3c_poly *qp, q3c_ipix_t *out_ipix_arr_fulls,
                   q3c_ipix_t *out_ipix_arr_partials, char *too_large)
{
    return q3c_region_query(hprm, Q3C_POLYGON, qp, out_ipix_arr_fulls, out_ipix_arr_partials, too_large);
}
