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


#include <string.h>

/* Postgres stuff */
#include "postgres.h"
#include "executor/spi.h"
#include "funcapi.h"      
#include "utils/lsyscache.h"
/* I included that just to remove the gcc warning 
 * q3c.c:128: warning: implicit declaration of function `get_typlenbyvalalign'
 */

/*
#include "fmgr.h"
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "access/heapam.h"  
#include "utils/builtins.h"
#include "utils/memutils.h"   
#include "catalog/pg_proc.h"
#include "utils/typcache.h"
*/

/* End of Postgres stuff */

#include "common.h"



/* Postgres functions */
Datum pgq3c_ang2ipix(PG_FUNCTION_ARGS);
Datum pgq3c_ang2ipix_real(PG_FUNCTION_ARGS);
Datum pgq3c_dist(PG_FUNCTION_ARGS);
Datum pgq3c_sindist(PG_FUNCTION_ARGS);
Datum q3c_strquery(PG_FUNCTION_ARGS);
Datum pgq3c_nearby_it(PG_FUNCTION_ARGS);
Datum pgq3c_ellipse_nearby_it(PG_FUNCTION_ARGS);
Datum pgq3c_radial_array(PG_FUNCTION_ARGS);
Datum pgq3c_radial_query_it(PG_FUNCTION_ARGS);
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
#ifdef Q3C_INT8
			return PointerGetDatum((&ipix_buf));
			//PG_RETURN_INT64(ipix_buf);
#endif
#ifdef Q3C_INT4
			PG_RETURN_INT32(ipix_buf);
#endif
		} 
	} 
	
	q3c_ang2ipix(&hprm, ra, dec, &ipix);
	
	ra_buf = ra;
	dec_buf = dec;
	ipix_buf = ipix;
	invocation=1;
	
	
#ifdef Q3C_INT8
	/*PG_RETURN_INT64(ipix);*/
	return PointerGetDatum((&ipix_buf));
#endif
#ifdef Q3C_INT4
	PG_RETURN_INT32(ipix);
#endif
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
#ifdef Q3C_INT8
			return PointerGetDatum((&ipix_buf));
			//PG_RETURN_INT64(ipix_buf);
#endif
#ifdef Q3C_INT4
			PG_RETURN_INT32(ipix_buf);
#endif
		} 
	} 
	
	q3c_ang2ipix(&hprm, ra, dec, &ipix);
	
	ra_buf = ra;
	dec_buf = dec;
	ipix_buf = ipix;
	invocation=1;
	
#ifdef Q3C_INT8
	/* PG_RETURN_INT64(ipix); */
	return PointerGetDatum((&ipix_buf));
#endif
#ifdef Q3C_INT4
	PG_RETURN_INT32(ipix);
#endif
	
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
#ifdef Q3C_INT4 
			PG_RETURN_INT32(ipix_array_buf[iteration]);
#endif
#ifdef Q3C_INT8 
			PG_RETURN_INT64(ipix_array_buf[iteration]);
#endif
		}
	}
	
	/*q3c_get_nearby(&hprm, ra_cen, dec_cen, radius, ipix_array);*/
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
#ifdef Q3C_INT4 
	PG_RETURN_INT32(ipix_array_buf[iteration]);
#endif
#ifdef Q3C_INT8 
	PG_RETURN_INT64(ipix_array_buf[iteration]);
#endif  
}



PG_FUNCTION_INFO_V1(pgq3c_ellipse_nearby_it);
Datum pgq3c_ellipse_nearby_it(PG_FUNCTION_ARGS)
{
	q3c_ipix_t ipix_array[8];
	static q3c_ipix_t ipix_array_buf[8];
	static q3c_coord_t ra_cen_buf, dec_cen_buf, radius_buf;
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
#ifdef Q3C_INT4 
			PG_RETURN_INT32(ipix_array_buf[iteration]);
#endif
#ifdef Q3C_INT8 
			PG_RETURN_INT64(ipix_array_buf[iteration]);
#endif
		}
	}
	
	/*q3c_get_nearby(&hprm, ra_cen, dec_cen, radius, ipix_array);*/
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
	
	invocation=1;
#ifdef Q3C_INT4 
	PG_RETURN_INT32(ipix_array_buf[iteration]);
#endif
#ifdef Q3C_INT8 
	PG_RETURN_INT64(ipix_array_buf[iteration]);
#endif  
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
	
#define n_partials 50
#define n_fulls 50
	static q3c_ipix_t partials[2 * n_partials];
	static q3c_ipix_t fulls[2 * n_fulls];
	/*  !!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!! 
	 * Here the n_partials and n_fulls is the number of pairs !!! of ranges  
	 * So we should have the array with the size twice bigger
	 */
 
#undef n_fulls
#undef n_partials

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
#ifdef Q3C_INT4 
			if (full_flag)
			{
				PG_RETURN_INT32(fulls[iteration]);
			}
			else
			{
				PG_RETURN_INT32(partials[iteration]);
			}
#endif
#ifdef Q3C_INT8 
			if (full_flag)
			{
				PG_RETURN_INT64(fulls[iteration]);
			}
			else
			{
				PG_RETURN_INT64(partials[iteration]);			
			}
#endif
		}
	}
		
	q3c_new_radial_query(&hprm, ra_cen, dec_cen, radius, fulls, partials);

	ra_cen_buf = ra_cen;
	dec_cen_buf = dec_cen;
	radius_buf = radius;
	invocation = 1;

#ifdef Q3C_INT4 
	if (full_flag)
	{
		PG_RETURN_INT32(fulls[iteration]);
	}
	else
	{
		PG_RETURN_INT32(partials[iteration]);			
	}
#endif
#ifdef Q3C_INT8
	if (full_flag)
	{
		PG_RETURN_INT64(fulls[iteration]);
	}
	else
	{
		PG_RETURN_INT64(partials[iteration]);			
	}
#endif
}



PG_FUNCTION_INFO_V1(pgq3c_poly_query_it);
Datum pgq3c_poly_query_it(PG_FUNCTION_ARGS)
{
	ArrayType *poly_arr = PG_GETARG_ARRAYTYPE_P(0);
	extern struct q3c_prm hprm;

	int iteration = PG_GETARG_INT32(1); /* iteration */
	int full_flag = PG_GETARG_INT32(2); /* full_flag */
	/* 1 means full, 0 means partial*/
	int16 typlen;
	bool typbyval;
	char typalign;
	int i;
	int poly_nitems;
	Oid element_type;
	char *p;
	
#define n_partials 50
#define n_fulls 50
	/*  !!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!! 
	 * Here the n_partials and n_fulls is the number of pairs !!! of ranges  
	 * So we should have the array with the size twice bigger
	 */    
	static q3c_ipix_t partials[2 * n_partials];
	static q3c_ipix_t fulls[2 * n_fulls];
	static q3c_poly qp;
#undef n_partials
#undef n_fulls


#define max_n_poly 100
	static q3c_coord_t ra[max_n_poly], dec[max_n_poly], x[max_n_poly],
			y[max_n_poly], ax[max_n_poly], ay[max_n_poly];
#undef max_n_poly

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
#ifdef Q3C_INT4 
			if (full_flag)
			{
				PG_RETURN_INT32(fulls[iteration]);
			}
			else
			{
				PG_RETURN_INT32(partials[iteration]);			
			}
#endif
#ifdef Q3C_INT8 
			if (full_flag)
			{
				PG_RETURN_INT64(fulls[iteration]);
			}
			else
			{
				PG_RETURN_INT64(partials[iteration]);			
			}
#endif
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
	
	poly_nitems /= 2;
	qp.n = poly_nitems;
	for (i = 0; i < poly_nitems; i++)
	{
		ra[i] = DatumGetFloat8(fetch_att(p, typbyval, typlen));
		p = att_addlength(p, typlen, PointerGetDatum(p));
		p = (char *) att_align(p, typalign);
		dec[i] = DatumGetFloat8(fetch_att(p, typbyval, typlen));
		p = att_addlength(p, typlen, PointerGetDatum(p));
		p = (char *) att_align(p, typalign);
	}
	
	qp.ra = ra;
	qp.dec = dec;
	qp.x = x;
	qp.y = y;
	qp.ax = ax;
	qp.ay = ay;

	/* fprintf(stderr,"%f %f %f %f",qp.ra[0],qp.dec[0],qp.ra[1],qp.dec[1]); */
	q3c_poly_query(&hprm, &qp, fulls, partials);
	
	invocation = 1;

#ifdef Q3C_INT4 
	if (full_flag)
	{
		PG_RETURN_INT32(fulls[iteration]);
	}
	else
	{
		PG_RETURN_INT32(partials[iteration]);			
	}
#endif
#ifdef Q3C_INT8
	if (full_flag)
	{
		PG_RETURN_INT64(fulls[iteration]);
	}
	else
	{
		PG_RETURN_INT64(partials[iteration]);			
	}
#endif
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
	
#define max_n_poly 100
	static q3c_coord_t in_ra[max_n_poly], in_dec[max_n_poly];
#undef max_n_poly
	static int invocation ;
	
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
		elog(ERROR, "Invalid polygon! Less then 3 vertexes");
	}
	
	p = ARR_DATA_PTR(poly_arr);
	poly_nitems /= 2;
	n = poly_nitems;
	invocation = 1;
	
	for (i = 0; i < poly_nitems; i++)
	{
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
	}
	
	result = (q3c_check_sphere_point_in_poly(&hprm, n, in_ra, in_dec,
											ra_cen, dec_cen, invocation)) > 0;
	
	PG_RETURN_BOOL((result));      
}


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 *                     Section of obsolete functions
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */


/* !!!!!!!!!!! OBSOLETE !!!!!!!!!!!!!!! */
#if 0
PG_FUNCTION_INFO_V1(q3c_strquery);
Datum q3c_strquery(PG_FUNCTION_ARGS)
{
	extern struct q3c_prm hprm;
	text *arg0 = PG_GETARG_TEXT_P(0);
	text *ra_col0 = PG_GETARG_TEXT_P(1);
	text *dec_col0 = PG_GETARG_TEXT_P(2);
	q3c_coord_t arg = PG_GETARG_FLOAT8(3);
	q3c_coord_t arg1 = PG_GETARG_FLOAT8(4);
	q3c_coord_t arg2 = PG_GETARG_FLOAT8(5);
	static char qstring[30000+VARHDRSZ];  
	static char tab_name[256], ra_col[256], dec_col[256];
	VarChar *tt;
	
	strncpy(tab_name,(char *)VARDATA(arg0),256);
	tab_name[VARSIZE(arg0)-VARHDRSZ]=0;
	
	strncpy(ra_col,(char *)VARDATA(ra_col0),256);
	ra_col[VARSIZE(ra_col0)-VARHDRSZ]=0;
	
	strncpy(dec_col,(char *)VARDATA(dec_col0),256);
	dec_col[VARSIZE(dec_col0)-VARHDRSZ]=0;
	
	tt = (VarChar *)(qstring);
	VARATT_SIZEP(tt) = 30000;
	
	/* not more then 30000 characters */
	q3c_radial_query(&hprm, tab_name, ra_col, dec_col, arg, arg1, arg2,
					qstring + VARHDRSZ);
	PG_RETURN_TEXT_P(tt);
	
}
#endif


/* !!!!!!!!!!!!!!!! OBSOLETE !!!!!!!!!!!!! */
#if 0
PG_FUNCTION_INFO_V1(pgq3c_nearby);
Datum pgq3c_nearby(PG_FUNCTION_ARGS)
{
  ArrayType  *result;
  Oid         element_type;
  q3c_ipix_t ipix_array[8];
  static q3c_ipix_t ipix_array_buf[8];
  static q3c_coord_t ra_cen_buf, dec_cen_buf, rad_buf;
  static int invocation;
  Datum array[8];
  int16       typlen;
  bool        typbyval;
  char        typalign;
  int         ndims;
  int         dims[MAXDIM];
  int         lbs[MAXDIM];
  int i;
  q3c_circle_region circle;
  
  extern struct q3c_prm hprm;
  q3c_coord_t ra_cen = PG_GETARG_FLOAT8(0); // ra_cen
  q3c_coord_t dec_cen = PG_GETARG_FLOAT8(1); // dec_cen
  q3c_coord_t rad = PG_GETARG_FLOAT8(2); // error radius


  if (invocation == 0)
  //* If this is the first invocation of the function */
  {
    invocation = 1;  
  }
  else
  {
    if ((ra_cen == ra_cen_buf) && (dec_cen == dec_cen_buf) && (rad == rad_buf))
    {
      for(i = 0;i < 8; i++)
      {
        ipix_array[i] = ipix_array_buf[i];
      }
    }
  }


#ifdef Q3C_INT4 
  element_type=INT4OID;
#endif
#ifdef Q3C_INT8 
  element_type=INT8OID;
#endif

  //q3c_get_nearby(&hprm,ra_cen,dec_cen,radius_cen,ipix_array);
  circle.ra = ra_cen;
  circle.dec = dec_cen;
  circle.rad = rad;
  
  q3c_get_nearby(&hprm, Q3C_CIRCLE, &circle, ipix_array);

  for(i = 0;i < 8; i++)
  {
    ipix_array_buf[i] = ipix_array[i];
  }
  ra_cen_buf=ra_cen;
  dec_cen_buf=dec_cen;
  rad_buf=rad;

  

  for (i = 0; i < 8; i++)
  {
#ifdef Q3C_INT4 
    array[i] = Int32GetDatum(ipix_array[i]);
#endif
#ifdef Q3C_INT8 
    array[i] = Int64GetDatum(ipix_array[i]);
#endif
  }

  /* we have one dimension */
  ndims = 1;
  /* and one element */
  dims[0] = 8;
  /* and lower bound is 1 */
  lbs[0] = 1;

  /* get required info about the element type */
  get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);

  /* now build the array */
  result = construct_md_array(array, ndims, dims, lbs,
                              element_type, typlen, typbyval, typalign);

  PG_RETURN_ARRAYTYPE_P(result);
}
#endif

/* !!!!!!!!!!!!OBSOLETE!!!!!!!! */
#if 0
PG_FUNCTION_INFO_V1(pgq3c_nearby_split);
Datum pgq3c_nearby_split(PG_FUNCTION_ARGS)
{
  ArrayType  *result;
  Oid         element_type;
  q3c_ipix_t input_array[2];
  Datum array[2];
  int16       typlen;
  bool        typbyval;
  char        typalign;
  int         ndims;
  int         dims[MAXDIM];
  int         lbs[MAXDIM];  
  
  extern struct q3c_prm hprm;
  q3c_coord_t arg0 = PG_GETARG_FLOAT8(0); // ra_cen
  q3c_coord_t arg1 = PG_GETARG_FLOAT8(1); // dec_cen
  q3c_coord_t arg2 = PG_GETARG_FLOAT8(2); // error radius
  int arg3 = PG_GETARG_INT32(3); //iteration number

#ifdef Q3C_INT4 
  element_type=INT4OID;
#endif
#ifdef Q3C_INT8 
  element_type=INT8OID;
#endif

  q3c_get_nearby_split(&hprm,arg0,arg1,arg2,input_array,arg3);   

#ifdef Q3C_INT4 
    array[0]=Int32GetDatum(input_array[0]);
    array[1]=Int32GetDatum(input_array[1]);
#endif
#ifdef Q3C_INT8 
    array[0]=Int64GetDatum(input_array[0]);
    array[1]=Int64GetDatum(input_array[1]);
#endif

  /* we have one dimension */
  ndims = 1;
  /* and one element */
  dims[0] = 2;
  /* and lower bound is 1 */
  lbs[0] = 1;

  /* get required info about the element type */
  get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);

  /* now build the array */
  result = construct_md_array(array, ndims, dims, lbs,
                              element_type, typlen, typbyval, typalign);

  PG_RETURN_ARRAYTYPE_P(result);
}
#endif

/* !!!!!!!!!!!! OBSOLETE !!!!!!!!!!!!! */
#if 0
PG_FUNCTION_INFO_V1(pgq3c_radial_array);
Datum pgq3c_radial_array(PG_FUNCTION_ARGS)
{
	ArrayType *partial_array_result;
	ArrayType *full_array_result;
	
	Oid element_type;
	//q3c_ipix_t input_array[2];
	Datum output_columns[2];
	int16 typlen;
	bool typbyval;
	char typalign;
	int ndims, dims[MAXDIM], lbs[MAXDIM];
	bool is_null[2]={0,0};
	extern struct q3c_prm hprm;
	q3c_coord_t ra0 = PG_GETARG_FLOAT8(0); // ra_cen
	q3c_coord_t dec0 = PG_GETARG_FLOAT8(1); // dec_cen
	q3c_coord_t rad = PG_GETARG_FLOAT8(2); // error radius
	TupleDesc       tupdesc;
	if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
	{
		elog(ERROR, "return type must be a row type");
	}
	
	BlessTupleDesc(tupdesc);
	
	int n_partials = 1600, n_fulls = 1600, i;
	
#ifdef Q3C_INT4 
	element_type=INT4OID;
#endif
#ifdef Q3C_INT8 
	element_type=INT8OID;
#endif

	/* q3c_get_nearby_split(&hprm,arg0,arg1,arg2,input_array,arg3);*/
	
#ifdef Q3C_INT4 
	/* array[0]=Int32GetDatum(input_array[0]);
	array[1]=Int32GetDatum(input_array[1]);
	*/
#endif
#ifdef Q3C_INT8 
	Datum *partial_array_ptr = palloc(n_partials * sizeof(int64 *));
	int64 *partial_array = palloc(n_partials * sizeof(int64));
	
	/*
	for(i = 0; i < n_partials; i++)
	{
		partial_array_ptr[i] = PointerGetDatum(partial_array + i * sizeof(int64));
	}
	*/
	
	Datum *full_array_ptr = palloc(n_fulls * sizeof(int64 *));
	int64 *full_array = palloc(n_fulls * sizeof(int64));
	
	/*
	for(i = 0; i < n_fulls; i++)
	{
		full_array_ptr[i] = PointerGetDatum(full_array + i * sizeof(int64));
	}
	*/  
	
	q3c_new_radial_query(&hprm, ra0, dec0, rad, (q3c_ipix_t *) full_array, (q3c_ipix_t *)partial_array);
	
	for(i = 0; i < n_partials; i++)
	{
		partial_array_ptr[i] = Int64GetDatum(partial_array[i]);
	}
	
	for(i = 0; i < n_fulls; i++)
	{
		full_array_ptr[i] = Int64GetDatum(full_array[i]);
	}

	//array[0]=Int64GetDatum(input_array[0]);
	//array[1]=Int64GetDatum(input_array[1]);
#endif
	
	/* we have one dimension */
	ndims = 1;
	/* and one element */
	dims[0] = n_partials;
	/* and lower bound is 1 */
	lbs[0] = 1;
	
	/* get required info about the element type */
	get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);
	
	/* now build the array */
	partial_array_result = construct_md_array(partial_array_ptr, ndims, dims, 
                                            lbs, element_type, typlen,
                                            typbyval, typalign);

	/* we have one dimension */
	ndims = 1;
	/* and one element */
	dims[0] = n_fulls;
	/* and lower bound is 1 */
	lbs[0] = 1;
	
	/* get required info about the element type */
	get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);
	
	/* now build the array */
	full_array_result = construct_md_array(full_array_ptr, ndims, dims, lbs,
                                         element_type, typlen, typbyval,
                                         typalign);
	
	output_columns[0] = PointerGetDatum(partial_array_result);
	output_columns[1] = PointerGetDatum(full_array_result);
	
	HeapTuple heaptup = heap_form_tuple (tupdesc, output_columns, is_null);
	pfree(partial_array_ptr);
	pfree(partial_array);
	pfree(full_array_ptr);
	pfree(full_array);
	
	return HeapTupleGetDatum(heaptup);
}
#endif
