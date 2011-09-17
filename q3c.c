/*
	   Copyright (C) 2004-2011 Sergey Koposov

    Author: Sergey Koposov, Sternberg Astronomical Institute
    						Institute of Astronomy, Cambridge
    Email: koposov@ast.cam.ac.uk
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


#include <string.h>

/* Postgres stuff */
#include "postgres.h"
#include "executor/spi.h"
#include "utils/lsyscache.h"
/* I included that just to remove the gcc warning
 * q3c.c:128: warning: implicit declaration of function `get_typlenbyvalalign'
 */
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "fmgr.h"

/* For PostgreSQL versions >= 8.2 */
#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif
/* End of Postgres stuff */

#include "common.h"

/* Postgres functions */
Datum pgq3c_ang2ipix(PG_FUNCTION_ARGS);
Datum pgq3c_ang2ipix_real(PG_FUNCTION_ARGS);
Datum pgq3c_ipix2ang(PG_FUNCTION_ARGS);
Datum pgq3c_pixarea(PG_FUNCTION_ARGS);
Datum pgq3c_dist(PG_FUNCTION_ARGS);
Datum pgq3c_sindist(PG_FUNCTION_ARGS);
Datum q3c_strquery(PG_FUNCTION_ARGS);
Datum pgq3c_nearby_it(PG_FUNCTION_ARGS);
Datum pgq3c_ellipse_nearby_it(PG_FUNCTION_ARGS);
Datum pgq3c_radial_array(PG_FUNCTION_ARGS);
Datum pgq3c_radial_query_it(PG_FUNCTION_ARGS);
Datum pgq3c_ellipse_query_it(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query_it(PG_FUNCTION_ARGS);
Datum pgq3c_in_ellipse(PG_FUNCTION_ARGS);
Datum pgq3c_in_poly(PG_FUNCTION_ARGS);


PG_FUNCTION_INFO_V1(pgq3c_ang2ipix);
Datum pgq3c_ang2ipix(PG_FUNCTION_ARGS)
{
	extern struct q3c_prm hprm;
	q3c_coord_t ra = PG_GETARG_FLOAT8(0);
	q3c_coord_t dec = PG_GETARG_FLOAT8(1);
	q3c_ipix_t ipix;
	static int invocation;
	static q3c_coord_t ra_buf, dec_buf;
	static q3c_ipix_t ipix_buf;

	if (invocation==0)
	{

	}
	else
	{
		if ((ra == ra_buf) && (dec == dec_buf))
		{
			/*return PointerGetDatum((&ipix_buf));*/
			PG_RETURN_INT64(ipix_buf);
		}
	}

	q3c_ang2ipix(&hprm, ra, dec, &ipix);

	ra_buf = ra;
	dec_buf = dec;
	ipix_buf = ipix;
	invocation=1;


	PG_RETURN_INT64(ipix);
	/*return PointerGetDatum((&ipix_buf));*/
}



PG_FUNCTION_INFO_V1(pgq3c_ang2ipix_real);
Datum pgq3c_ang2ipix_real(PG_FUNCTION_ARGS)
{
	extern struct q3c_prm hprm;
	q3c_coord_t ra = PG_GETARG_FLOAT4(0);
	q3c_coord_t dec = PG_GETARG_FLOAT4(1);
	q3c_ipix_t ipix;
	static int invocation;
	static q3c_coord_t ra_buf, dec_buf;
	static q3c_ipix_t ipix_buf;

	//fprintf(stderr,"Y");

	if (invocation==0)
	{

	}
	else
	{
		if ((ra == ra_buf) && (dec == dec_buf))
		{
			/*return PointerGetDatum((&ipix_buf));*/
			PG_RETURN_INT64(ipix_buf);
		}
	}

	q3c_ang2ipix(&hprm, ra, dec, &ipix);

	ra_buf = ra;
	dec_buf = dec;
	ipix_buf = ipix;
	invocation=1;

	PG_RETURN_INT64(ipix);
	/* return PointerGetDatum((&ipix_buf)); */

}

PG_FUNCTION_INFO_V1(pgq3c_ipix2ang);
Datum pgq3c_ipix2ang(PG_FUNCTION_ARGS)
{
	extern struct q3c_prm hprm;
	q3c_ipix_t ipix;
	q3c_coord_t ra, dec;

	Datum       *data;
	int16       typlen;
	bool        typbyval;
	char        typalign;
	int         ndims, dims[MAXDIM], lbs[MAXDIM];
	ArrayType  *result;
	ipix = PG_GETARG_INT64(0);

	q3c_ipix2ang(&hprm, ipix, &ra, &dec);

    /* we have one dimension */
    ndims = 1;
    /* and one element */
    dims[0] = 2;
    /* and lower bound is 1 */
    lbs[0] = 0;
	data = ( Datum *) palloc(sizeof(Datum)*2);
	data[0] = Float8GetDatum (ra);
	data[1] = Float8GetDatum (dec);
    /* get required info about the element type */
    get_typlenbyvalalign(FLOAT8OID, &typlen, &typbyval, &typalign);

    /* now build the array */
    result = construct_array(data, 2, FLOAT8OID, typlen, typbyval, typalign);

    PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(pgq3c_pixarea);
Datum pgq3c_pixarea(PG_FUNCTION_ARGS)
{
	extern struct q3c_prm hprm;
	q3c_ipix_t ipix;
	q3c_coord_t res;
	int depth;

	ipix = PG_GETARG_INT64(0);

	depth = PG_GETARG_INT32(1);

	if (depth <= 0)
	{
		elog(ERROR, "Invalid depth. It should be greater than 0.");
	}
	if (depth > 30)
	{
		elog(ERROR, "Invalid depth. It should be less than 30.");
	}

	res = q3c_pixarea(&hprm, ipix, depth);

	PG_RETURN_FLOAT8(res);
}


PG_FUNCTION_INFO_V1(pgq3c_dist);
Datum pgq3c_dist(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra1 = PG_GETARG_FLOAT8(0);
	q3c_coord_t dec1 = PG_GETARG_FLOAT8(1);
	q3c_coord_t ra2 = PG_GETARG_FLOAT8(2);
	q3c_coord_t dec2 = PG_GETARG_FLOAT8(3);
	q3c_coord_t res;
	res = q3c_dist(ra1, dec1, ra2, dec2);
	PG_RETURN_FLOAT8(res);
}

PG_FUNCTION_INFO_V1(pgq3c_sindist);
Datum pgq3c_sindist(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra1 = PG_GETARG_FLOAT8(0);
	q3c_coord_t dec1 = PG_GETARG_FLOAT8(1);
	q3c_coord_t ra2 = PG_GETARG_FLOAT8(2);
	q3c_coord_t dec2 = PG_GETARG_FLOAT8(3);
	q3c_coord_t res;
	res = q3c_sindist(ra1, dec1, ra2, dec2);
	PG_RETURN_FLOAT8(res);
}



PG_FUNCTION_INFO_V1(pgq3c_nearby_it);
Datum pgq3c_nearby_it(PG_FUNCTION_ARGS)
{
	q3c_ipix_t ipix_array[8];
	static q3c_ipix_t ipix_array_buf[8];
	static q3c_coord_t ra_cen_buf, dec_cen_buf, radius_buf;
	static int invocation;
	int i;
	extern struct q3c_prm hprm;
	q3c_circle_region circle;

	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(0); // ra_cen
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(1); // dec_cen
	q3c_coord_t radius = PG_GETARG_FLOAT8(2); // error radius
	int iteration = PG_GETARG_INT32(3); // iteration
	
	if ( (!isfinite(ra_cen)) || (!isfinite(dec_cen)) )
	{
		elog(ERROR, "The values of ra,dec are infinites or NaNs");
	}

	if (invocation == 0)
	/* If this is the first invocation of the function */
	{
	/* I should set invocation=1 ONLY!!! after setting ra_cen_buf, dec_cen_buf and
   	 * ipix_buf. Because if the program will be canceled or crashed
	 * for some reason the invocation should be == 0
 	 */
	}
	else
	{
		if ((ra_cen == ra_cen_buf) && (dec_cen == dec_cen_buf) && (radius == radius_buf))
		{
			PG_RETURN_INT64(ipix_array_buf[iteration]);
		}
	}

	/*q3c_get_nearby(&hprm, ra_cen, dec_cen, radius, ipix_array);*/
	ra_cen = UNWRAP_RA(ra_cen);
	if (q3c_fabs(dec_cen)>90) {dec_cen = q3c_fmod(dec_cen,90);}
	circle.ra = ra_cen;
	circle.dec = dec_cen;
	circle.rad = radius;
	q3c_get_nearby(&hprm, Q3C_CIRCLE, &circle, ipix_array);

	for(i = 0; i < 8; i++)
	{
		ipix_array_buf[i] = ipix_array[i];
	}

	ra_cen_buf = ra_cen;
	dec_cen_buf = dec_cen;
	radius_buf = radius;

	invocation=1;
	PG_RETURN_INT64(ipix_array_buf[iteration]);
}



PG_FUNCTION_INFO_V1(pgq3c_ellipse_nearby_it);
Datum pgq3c_ellipse_nearby_it(PG_FUNCTION_ARGS)
{
	q3c_ipix_t ipix_array[8];
	static q3c_ipix_t ipix_array_buf[8];
	static q3c_coord_t ra_cen_buf, dec_cen_buf, radius_buf, axis_ratio_buf, PA_buf;
	static int invocation;
	int i;
	q3c_ellipse_region ellipse;

	extern struct q3c_prm hprm;
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(0); /* ra_cen */
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(1); /* dec_cen */
	q3c_coord_t radius = PG_GETARG_FLOAT8(2); /* error radius */
	q3c_coord_t axis_ratio = PG_GETARG_FLOAT8(3); /* axis_ratio */
	q3c_coord_t PA = PG_GETARG_FLOAT8(4); /* PA */
	int iteration = PG_GETARG_INT32(5); /* iteration */

	if ( (!isfinite(ra_cen)) || (!isfinite(dec_cen)) || (!isfinite(radius)) )
	{
		elog(ERROR, "The values of ra,dec,radius are infinites or NaNs");
	}


	if (invocation == 0)
	/* If this is the first invocation of the function */
	{
	/* I should set invocation=1 ONLY!!! after setting ra_cen_buf, dec_cen_buf and
	 * ipix_buf. Because if the program will be canceled or crashed
	 * for some reason the invocation should be == 0
	 */
	}
	else
	{
		if ((ra_cen == ra_cen_buf) && (dec_cen == dec_cen_buf) &&
			(radius == radius_buf) && (PA == PA_buf) &&
			(axis_ratio == axis_ratio_buf))
		{
			PG_RETURN_INT64(ipix_array_buf[iteration]);
		}
	}

	/*q3c_get_nearby(&hprm, ra_cen, dec_cen, radius, ipix_array);*/
	ra_cen = UNWRAP_RA(ra_cen);
	if (q3c_fabs(dec_cen)>90) {dec_cen = q3c_fmod(dec_cen,90);}

	ellipse.ra = ra_cen;
	ellipse.dec = dec_cen;
	ellipse.rad = radius;
	ellipse.e = q3c_sqrt ( 1 - axis_ratio * axis_ratio );
	ellipse.PA = PA;

	q3c_get_nearby(&hprm, Q3C_ELLIPSE, &ellipse, ipix_array);

	for(i = 0; i < 8; i++)
	{
		ipix_array_buf[i] = ipix_array[i];
	}

	ra_cen_buf = ra_cen;
	dec_cen_buf = dec_cen;
	radius_buf = radius;
	axis_ratio_buf = axis_ratio;
	PA_buf = PA;

	invocation=1;
	PG_RETURN_INT64(ipix_array_buf[iteration]);
}






PG_FUNCTION_INFO_V1(pgq3c_radial_query_it);
Datum pgq3c_radial_query_it(PG_FUNCTION_ARGS)
{
	extern struct q3c_prm hprm;
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(0);
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(1);
	q3c_coord_t radius = PG_GETARG_FLOAT8(2); /* error radius */
	int iteration = PG_GETARG_INT32(3); /* iteration */
	int full_flag = PG_GETARG_INT32(4); /* full_flag */
	/* 1 means full, 0 means partial */

	/* const int n_partials = 800, n_fulls = 800;*/



	static q3c_coord_t ra_cen_buf, dec_cen_buf, radius_buf;
	/* static q3c_ipix_t partials[2 * n_partials]; */
	/* static q3c_ipix_t fulls[2 * n_fulls]; */

	static q3c_ipix_t partials[2 * Q3C_NPARTIALS];
	static q3c_ipix_t fulls[2 * Q3C_NFULLS];
	/*  !!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!
	 * Here the Q3C_NPARTIALS and Q3C_NFULLS is the number of pairs !!! of ranges
	 * So we should have the array with the size twice bigger
	 */


	static int invocation;

	if (invocation == 0)
	/* If this is the first invocation of the function */
	{
	/* I should set invocation=1 ONLY!!! after setting ra_cen_buf, dec_cen_buf and
	 * ipix_buf. Because if the program will be canceled or crashed
	 * for some reason the invocation should be == 0
	 */
	}
	else
	{
		if ((ra_cen == ra_cen_buf) && (dec_cen == dec_cen_buf) && (radius == radius_buf))
		{
			if (full_flag)
			{
				PG_RETURN_INT64(fulls[iteration]);
			}
			else
			{
				PG_RETURN_INT64(partials[iteration]);
			}
		}
	}

	q3c_new_radial_query(&hprm, ra_cen, dec_cen, radius, fulls, partials);

	ra_cen_buf = ra_cen;
	dec_cen_buf = dec_cen;
	radius_buf = radius;
	invocation = 1;

	if (full_flag)
	{
		PG_RETURN_INT64(fulls[iteration]);
	}
	else
	{
		PG_RETURN_INT64(partials[iteration]);
	}
}


PG_FUNCTION_INFO_V1(pgq3c_ellipse_query_it);
Datum pgq3c_ellipse_query_it(PG_FUNCTION_ARGS)
{
	extern struct q3c_prm hprm;
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(0);
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(1);
	q3c_coord_t radius = PG_GETARG_FLOAT8(2); /* Major axis */
	q3c_coord_t axis_ratio = PG_GETARG_FLOAT8(3); /* Axis ratio */
	q3c_coord_t PA = PG_GETARG_FLOAT8(4); /* PA */
	int iteration = PG_GETARG_INT32(5); /* iteration */
	int full_flag = PG_GETARG_INT32(6); /* full_flag */
	q3c_coord_t ell = q3c_sqrt ( 1 - axis_ratio * axis_ratio );
	/* 1 means full, 0 means partial */

	/* const int n_partials = 800, n_fulls = 800;*/

	static q3c_coord_t ra_cen_buf, dec_cen_buf, radius_buf;
	/* static q3c_ipix_t partials[2 * n_partials]; */
	/* static q3c_ipix_t fulls[2 * n_fulls]; */

	static q3c_ipix_t partials[2 * Q3C_NPARTIALS];
	static q3c_ipix_t fulls[2 * Q3C_NFULLS];
	/*  !!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!
	 * Here the Q3C_NPARTIALS and Q3C_NFULLS is the number of pairs !!! of ranges
	 * So we should have the array with the size twice bigger
	 */

	static int invocation;

	if (invocation == 0)
	/* If this is the first invocation of the function */
	{
	/* I should set invocation=1 ONLY!!! after setting ra_cen_buf, dec_cen_buf and
	 * ipix_buf. Because if the program will be canceled or crashed
	 * for some reason the invocation should be == 0
	 */
	}
	else
	{
		if ((ra_cen == ra_cen_buf) && (dec_cen == dec_cen_buf) && (radius == radius_buf))
		{
			if (full_flag)
			{
				PG_RETURN_INT64(fulls[iteration]);
			}
			else
			{
				PG_RETURN_INT64(partials[iteration]);
			}
		}
	}

	q3c_ellipse_query(&hprm, ra_cen, dec_cen, radius, ell, PA, fulls,
		partials);

	ra_cen_buf = ra_cen;
	dec_cen_buf = dec_cen;
	radius_buf = radius;
	invocation = 1;

	if (full_flag)
	{
		PG_RETURN_INT64(fulls[iteration]);
	}
	else
	{
		PG_RETURN_INT64(partials[iteration]);
	}
}





PG_FUNCTION_INFO_V1(pgq3c_poly_query_it);
Datum pgq3c_poly_query_it(PG_FUNCTION_ARGS)
{
	ArrayType *poly_arr = PG_GETARG_ARRAYTYPE_P(0);
	extern struct q3c_prm hprm;

	int iteration = PG_GETARG_INT32(1); /* iteration */
	int full_flag = PG_GETARG_INT32(2); /* full_flag */
	char too_large = 0;
	/* 1 means full, 0 means partial*/
	int16 typlen;
	bool typbyval;
	char typalign;
	int i;
	int poly_nitems;
	Oid element_type;
	char *p;
#if PG_VERSION_NUM >= 80300
	bits8 *bitmap;
	int bitmask;
#endif

	/*  !!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!
	 * Here the Q3C_NPARTIALS and Q3C_NFULLS is the number of pairs !!! of ranges
	 * So we should have the array with the size twice bigger
	 */
	static q3c_ipix_t partials[2 * Q3C_NPARTIALS];
	static q3c_ipix_t fulls[2 * Q3C_NFULLS];
	static q3c_poly qp;

	static q3c_coord_t ra[Q3C_MAX_N_POLY_VERTEX], dec[Q3C_MAX_N_POLY_VERTEX],
		x[Q3C_MAX_N_POLY_VERTEX], y[Q3C_MAX_N_POLY_VERTEX],
		ax[Q3C_MAX_N_POLY_VERTEX], ay[Q3C_MAX_N_POLY_VERTEX];

	static int invocation;

	if (invocation == 0)
	/* If this is the first invocation of the function */
	{
	/* I should set invocation=1 ONLY!!! after setting ra_cen_buf, dec_cen_buf and
	 * ipix_buf. Because if the program will be canceled or crashed
	 * for some reason the invocation should be == 0
	 */
	}
	else
	{
		/* TODO !!!!!!!!!! */
		/* bad realization currently .... */
		/* Probably I should check that the polygon is the same ... */
		if (iteration > 0)
		{
			if (full_flag)
			{
				PG_RETURN_INT64(fulls[iteration]);
			}
			else
			{
				PG_RETURN_INT64(partials[iteration]);
			}
		}
	}

	poly_nitems = ArrayGetNItems(ARR_NDIM(poly_arr), ARR_DIMS(poly_arr));
	element_type = FLOAT8OID;

	get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);

/* Taken from /pgsql/src/backend/utils/adt/arrayfuncs.c
 function deconstruct_array*/

	p = ARR_DATA_PTR(poly_arr);

	if (poly_nitems % 2 != 0)
	{
		elog(ERROR, "Invalid array argument! \n The array should contain even number of arguments");
	}
	else if (poly_nitems <=4)
	{
		elog(ERROR, "Invalid polygon! Less then 4 vertexes");
	}
	else if (poly_nitems > 2 * Q3C_MAX_N_POLY_VERTEX)
	{
		elog(ERROR, "Q3C does not support polygons with number of vertices > %d", Q3C_MAX_N_POLY_VERTEX);
	}

	poly_nitems /= 2;
	qp.n = poly_nitems;
#if PG_VERSION_NUM >= 80300

	bitmap = ARR_NULLBITMAP(poly_arr);
	bitmask=1;
#endif

	for (i = 0; i < poly_nitems; i++)
	{
#if PG_VERSION_NUM >= 80300
		if (bitmap && (*bitmap & bitmask) == 0)
		{
			ereport(ERROR,
					(errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
					errmsg("null array element not allowed in this context")));
		}
		ra[i] = DatumGetFloat8(fetch_att(p, typbyval, typlen));
		p = att_addlength_pointer(p, typlen, PointerGetDatum(p));
		p = (char *) att_align_nominal(p, typalign);
		if (bitmap)
		{
			bitmask <<= 1;
			if (bitmask == 0x100)
			{
				bitmap++;
				bitmask = 1;
			}
		}
		if (bitmap && (*bitmap & bitmask) == 0)
		{
			ereport(ERROR,
					(errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
					errmsg("null array element not allowed in this context")));
		}

		dec[i] = DatumGetFloat8(fetch_att(p, typbyval, typlen));
		p = att_addlength_pointer(p, typlen, PointerGetDatum(p));
		p = (char *) att_align_nominal(p, typalign);
		if (bitmap)
		{
			bitmask <<= 1;
			if (bitmask == 0x100)
			{
				bitmap++;
				bitmask = 1;
			}
		}
#else
		ra[i] = DatumGetFloat8(fetch_att(p, typbyval, typlen));
		p = att_addlength(p, typlen, PointerGetDatum(p));
		p = (char *) att_align(p, typalign);
		dec[i] = DatumGetFloat8(fetch_att(p, typbyval, typlen));
		p = att_addlength(p, typlen, PointerGetDatum(p));
		p = (char *) att_align(p, typalign);
#endif
	}

	qp.ra = ra;
	qp.dec = dec;
	qp.x = x;
	qp.y = y;
	qp.ax = ax;
	qp.ay = ay;

	/* fprintf(stderr,"%f %f %f %f",qp.ra[0],qp.dec[0],qp.ra[1],qp.dec[1]); */
	q3c_poly_query(&hprm, &qp, fulls, partials, &too_large);
	if (too_large)
	{
		elog(ERROR, "The polygon is too large. Polygons having diameter >~23 degrees are unsupported");
	}
	invocation = 1;

	if (full_flag)
	{
		PG_RETURN_INT64(fulls[iteration]);
	}
	else
	{
		PG_RETURN_INT64(partials[iteration]);
	}
}


PG_FUNCTION_INFO_V1(pgq3c_in_ellipse);
Datum pgq3c_in_ellipse(PG_FUNCTION_ARGS)
{

	q3c_coord_t ra = PG_GETARG_FLOAT8(0); // ra_cen
	q3c_coord_t dec = PG_GETARG_FLOAT8(1); // dec_cen
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(2); // ra_cen
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(3); // dec_cen
	q3c_coord_t radius = PG_GETARG_FLOAT8(4); // error radius
	q3c_coord_t axis_ratio = PG_GETARG_FLOAT8(5); // axis_ratio
	q3c_coord_t PA = PG_GETARG_FLOAT8(6); // PA
	q3c_coord_t e = q3c_sqrt(1 - axis_ratio * axis_ratio);
	bool result = q3c_in_ellipse(ra_cen, dec_cen, ra,dec, radius, e, PA);
	PG_RETURN_BOOL(result);
}


PG_FUNCTION_INFO_V1(pgq3c_in_poly);
Datum pgq3c_in_poly(PG_FUNCTION_ARGS)
{
	extern struct q3c_prm hprm;

	static q3c_coord_t in_ra[Q3C_MAX_N_POLY_VERTEX], in_dec[Q3C_MAX_N_POLY_VERTEX];

	static int invocation ;
	char too_large;
	ArrayType *poly_arr = PG_GETARG_ARRAYTYPE_P(2); // ra_cen
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(0); // ra_cen
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(1); // dec_cen
	int16 typlen;
	bool typbyval;
	char typalign;

	int poly_nitems = ArrayGetNItems(ARR_NDIM(poly_arr), ARR_DIMS(poly_arr));
	int n, i;
	q3c_coord_t ra_cur, dec_cur;
	Oid element_type=FLOAT8OID;
	char *p;
	bool result;

#if PG_VERSION_NUM >= 80300
	bits8 *bitmap;
	int bitmask;
#endif


	get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);

	/* Taken from /pgsql/src/backend/utils/adt/arrayfuncs.c
	 * function deconstruct_array
	 */

	 if (poly_nitems % 2 != 0)
	 {
	 	elog(ERROR, "Invalid array argument! \n The array should contain even number of arguments");
	}
	else if (poly_nitems <= 4)
	{
		elog(ERROR, "Invalid polygon! Less than 3 vertexes");
	}

	p = ARR_DATA_PTR(poly_arr);
	poly_nitems /= 2;
	n = poly_nitems;
	invocation = 1;

#if PG_VERSION_NUM >= 80300

	bitmap = ARR_NULLBITMAP(poly_arr);
	bitmask=1;
#endif


	for (i = 0; i < poly_nitems; i++)
	{
#if PG_VERSION_NUM >= 80300
		if (bitmap && (*bitmap & bitmask) == 0)
		{
			ereport(ERROR,
					(errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
					errmsg("null array element not allowed in this context")));
		}
		ra_cur = DatumGetFloat8(fetch_att(p, typbyval, typlen));
		p = att_addlength_pointer(p, typlen, PointerGetDatum(p));
		p = (char *) att_align_nominal(p, typalign);
		if (bitmap)
		{
			bitmask <<= 1;
			if (bitmask == 0x100)
			{
				bitmap++;
				bitmask = 1;
			}
		}
		if (in_ra[i] != ra_cur)
		{
			invocation = 0;
			in_ra[i] = ra_cur;
		}
		if (bitmap && (*bitmap & bitmask) == 0)
		{
			ereport(ERROR,
					(errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
					errmsg("null array element not allowed in this context")));
		}

		dec_cur = DatumGetFloat8(fetch_att(p, typbyval, typlen));
		p = att_addlength_pointer(p, typlen, PointerGetDatum(p));
		p = (char *) att_align_nominal(p, typalign);
		if (bitmap)
		{
			bitmask <<= 1;
			if (bitmask == 0x100)
			{
				bitmap++;
				bitmask = 1;
			}
		}
		if (in_dec[i] != dec_cur)
		{
			invocation = 0;
			in_dec[i] = dec_cur;
		}

#else
		ra_cur  = DatumGetFloat8(fetch_att(p, typbyval, typlen));
		if (in_ra[i] != ra_cur)
		{
			invocation = 0;
			in_ra[i] = ra_cur;
		}
		p = att_addlength(p, typlen, PointerGetDatum(p));
		p = (char *) att_align(p, typalign);
		dec_cur = DatumGetFloat8(fetch_att(p, typbyval, typlen));
		if (in_dec[i] != dec_cur)
		{
			invocation = 0;
			in_dec[i] = dec_cur;
		}

		p = att_addlength(p, typlen, PointerGetDatum(p));
		p = (char *) att_align(p, typalign);
#endif
	}

	result = (q3c_check_sphere_point_in_poly(&hprm, n, in_ra, in_dec,
											ra_cen, dec_cen, &too_large, invocation)) !=
												Q3C_DISJUNCT;
	if (too_large)
	{
		elog(ERROR, "The polygon is too large. Polygons having diameter >~23 degrees are unsupported");
	}

	PG_RETURN_BOOL((result));
}
