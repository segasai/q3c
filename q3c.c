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


#include <string.h>

/* Postgres stuff */
#include "postgres.h"
#include "executor/spi.h"
#include "utils/lsyscache.h"
/* I included that just to remove the gcc warning
 * q3c.c:128: warning: implicit declaration of function `get_typlenbyvalalign'
 */
#include "utils/array.h"
#include "utils/geo_decls.h"
#include "catalog/pg_type.h"
#include "fmgr.h"
#if PG_VERSION_NUM >= 90300
#include "access/tupmacs.h"
#endif
//#include "nodes/relation.h"
#include "utils/selfuncs.h"



/* For PostgreSQL versions >= 8.2 */
#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif
/* End of Postgres stuff */

#include "common.h"

extern Node *estimate_expression_value(PlannerInfo *root, Node *node);

/* Postgres functions */
Datum pgq3c_ang2ipix(PG_FUNCTION_ARGS);
Datum pgq3c_ang2ipix_real(PG_FUNCTION_ARGS);
Datum pgq3c_ipix2ang(PG_FUNCTION_ARGS);
Datum pgq3c_pixarea(PG_FUNCTION_ARGS);
Datum pgq3c_dist(PG_FUNCTION_ARGS);
Datum pgq3c_dist_pm(PG_FUNCTION_ARGS);
Datum pgq3c_sindist(PG_FUNCTION_ARGS);
Datum pgq3c_sindist_pm(PG_FUNCTION_ARGS);
Datum q3c_strquery(PG_FUNCTION_ARGS);
Datum pgq3c_nearby_it(PG_FUNCTION_ARGS);
Datum pgq3c_nearby_pm_it(PG_FUNCTION_ARGS);
Datum pgq3c_ellipse_nearby_it(PG_FUNCTION_ARGS);
Datum pgq3c_radial_array(PG_FUNCTION_ARGS);
Datum pgq3c_radial_query_it(PG_FUNCTION_ARGS);
Datum pgq3c_ellipse_query_it(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query_it(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query1_it(PG_FUNCTION_ARGS);
Datum pgq3c_in_ellipse(PG_FUNCTION_ARGS);
Datum pgq3c_in_poly(PG_FUNCTION_ARGS);
Datum pgq3c_in_poly1(PG_FUNCTION_ARGS);

Datum pgq3c_get_version(PG_FUNCTION_ARGS);
Datum pgq3c_sel(PG_FUNCTION_ARGS);
Datum pgq3c_seljoin(PG_FUNCTION_ARGS);
Datum pgq3c_seloper(PG_FUNCTION_ARGS);


/* Dummy function that implements the selectivity operator */
PG_FUNCTION_INFO_V1(pgq3c_seloper);
Datum pgq3c_seloper(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(true);
}

/* The actual selectivity function, it returns the ratio of the
 * search circle to the whole sky area
 */
PG_FUNCTION_INFO_V1(pgq3c_sel);
Datum pgq3c_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	List   *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	Node   *left;
	Node *other;
	VariableStatData vardata;
	Datum radDatum;
	bool isnull;
	double rad;
	double ratio;

	/* this needs more protections against crazy inputs */
	if (list_length(args) != 2) { elog(ERROR, "Wrong inputs to selectivity function");}
	left = (Node *) linitial(args);

	examine_variable(root, left, varRelid, &vardata);
	other = estimate_expression_value(root, vardata.var);
	radDatum = ((Const *) other)->constvalue;
	isnull = ((Const *) other)->constisnull;
	/* We shouldn't be really getting null inputs here */
	if (!isnull)
	{
		rad = DatumGetFloat8(radDatum);
	}
	else
	{
		rad = 0;
	}
	ratio = 3.14 * rad * rad/41252. ; /* pi*r^2/whole_sky_area */

	/* clamp at 0, 1*/
	CLAMP_PROBABILITY(ratio);

	//elog(WARNING, "HERE0.... %e", ratio);

	PG_RETURN_FLOAT8(ratio);
}


PG_FUNCTION_INFO_V1(pgq3c_seljoin);
Datum pgq3c_seljoin(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	List   *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = 0;
	/* Because there is no varrelid in the join selectivity call
         * I just set it to zero */
	Node   *left;
	Node *other;
	VariableStatData vardata;
	Datum radDatum;
	bool isnull;
	double rad;
	double ratio;

	/* this needs more protections against crazy inputs */
	if (list_length(args) != 2) { elog(ERROR, "Wrong inputs to selectivity function");}
	left = (Node *) linitial(args);

	examine_variable(root, left, varRelid, &vardata);
	other = estimate_expression_value(root, vardata.var);
	radDatum = ((Const *) other)->constvalue;
	isnull = ((Const *) other)->constisnull;
	/* We shouldn't be really getting null inputs here */
	if (!isnull)
	{
		rad = DatumGetFloat8(radDatum);
	}
	else
	{
		rad = 0;
	}
	ratio = 3.14 * rad * rad/41252. ; /* pi*r^2/whole_sky_area */

	/* clamp at 0, 1*/
	CLAMP_PROBABILITY(ratio);

	PG_RETURN_FLOAT8(ratio);
}

static int convert_pgarray2poly(ArrayType *poly_arr, q3c_coord_t *in_ra, q3c_coord_t *in_dec, int *nvert);


PG_FUNCTION_INFO_V1(pgq3c_get_version);
Datum pgq3c_get_version(PG_FUNCTION_ARGS)
{
	char VERSION_MAX_BYTES = 100;
	char *buf = palloc(VERSION_MAX_BYTES);
	q3c_get_version(buf, VERSION_MAX_BYTES);
	PG_RETURN_CSTRING(buf);
}



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
			PG_RETURN_INT64(ipix_buf);
		}
	}
	if ((!isfinite(ra)) || (!isfinite(dec)))
	{
		PG_RETURN_NULL();
	}
	q3c_ang2ipix(&hprm, ra, dec, &ipix);

	ra_buf = ra;
	dec_buf = dec;
	ipix_buf = ipix;
	invocation=1;

	PG_RETURN_INT64(ipix);
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

	if (invocation==0)
	{

	}
	else
	{
		if ((ra == ra_buf) && (dec == dec_buf))
		{
			PG_RETURN_INT64(ipix_buf);
		}
	}
	if ((!isfinite(ra)) || (!isfinite(dec)))
	{
		PG_RETURN_NULL();
	}
	q3c_ang2ipix(&hprm, ra, dec, &ipix);

	ra_buf = ra;
	dec_buf = dec;
	ipix_buf = ipix;
	invocation=1;

	PG_RETURN_INT64(ipix);
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
	ArrayType  *result;
	ipix = PG_GETARG_INT64(0);

	q3c_ipix2ang(&hprm, ipix, &ra, &dec);

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
		elog(ERROR, "Invalid depth. It should be less than 31.");
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


PG_FUNCTION_INFO_V1(pgq3c_sindist_pm);
Datum pgq3c_sindist_pm(PG_FUNCTION_ARGS)
{
	q3c_coord_t pmra1, pmdec1, epoch1, epoch2;
	q3c_coord_t ra1, dec1, ra2, dec2, ra1_shift, dec1_shift, cdec;
	bool pm_enabled = true, cosdec;
	q3c_coord_t res;
	const int ra_arg_pos=0, dec_arg_pos=1, pmra_arg_pos=2, pmdec_arg_pos=3,
	  cosdec_arg_pos=4, epoch_arg_pos=5, ra2_arg_pos=6, dec2_arg_pos=7,
	  epoch2_arg_pos=8;

	if (PG_ARGISNULL(ra_arg_pos) || PG_ARGISNULL(dec_arg_pos) ||
		PG_ARGISNULL(ra2_arg_pos) || PG_ARGISNULL(dec2_arg_pos))
	{
		elog(ERROR, "The RA, DEC columns are not allowed to be null");
	}
	
	ra1 = PG_GETARG_FLOAT8(ra_arg_pos);
	dec1 = PG_GETARG_FLOAT8(dec_arg_pos);

	if (!PG_ARGISNULL(pmra_arg_pos)&&(!PG_ARGISNULL(pmdec_arg_pos))&&
	    (!PG_ARGISNULL(epoch_arg_pos))&&(!PG_ARGISNULL(epoch2_arg_pos)))
	{
	    pmra1 = PG_GETARG_FLOAT8(pmra_arg_pos);
	    pmdec1 = PG_GETARG_FLOAT8(pmdec_arg_pos);
	    epoch1 = PG_GETARG_FLOAT8(epoch_arg_pos);
	    epoch2 = PG_GETARG_FLOAT8(epoch2_arg_pos);
	}
	else
	{
		pm_enabled = false;
		pmra1 = 0;
		pmdec1 = 0;
		epoch1 = 0;
		epoch2 = 0;
	}
	cosdec = PG_GETARG_BOOL(cosdec_arg_pos);
	ra2 = PG_GETARG_FLOAT8(ra2_arg_pos);
	dec2 = PG_GETARG_FLOAT8(dec2_arg_pos);


    if (pm_enabled)
	{
	  if (cosdec)
	    {
	      cdec = cos(dec1*Q3C_DEGRA);
	    }
	  else
	    {
	      cdec = 1;
	    }
		ra1_shift = ra1 + pmra1 * (epoch2 - epoch1) / cdec / 3600000;
		dec1_shift = dec1 + pmdec1 * (epoch2 - epoch1) / 3600000;
	}
	else
	{
		ra1_shift = ra1;
		dec1_shift = dec1;
	}
	res = q3c_sindist(ra1_shift, dec1_shift, ra2, dec2);
	PG_RETURN_FLOAT8(res);
}


PG_FUNCTION_INFO_V1(pgq3c_dist_pm);
Datum pgq3c_dist_pm(PG_FUNCTION_ARGS)
{
	q3c_coord_t pmra1, pmdec1, epoch1, epoch2;
	q3c_coord_t ra1, dec1, ra2, dec2, ra1_shift, dec1_shift, cdec=1;
	bool pm_enabled = true, cosdec;
	q3c_coord_t res;

	const int ra_arg_pos=0, dec_arg_pos=1, pmra_arg_pos=2, pmdec_arg_pos=3,
	  cosdec_arg_pos=4, epoch_arg_pos=5, ra2_arg_pos=6, dec2_arg_pos=7,
	  epoch2_arg_pos=8;

	if (PG_ARGISNULL(ra_arg_pos) || PG_ARGISNULL(dec_arg_pos) ||
		PG_ARGISNULL(ra2_arg_pos) || PG_ARGISNULL(dec2_arg_pos))
	{
		elog(ERROR, "The RA, DEC columns are not allowed to be null");
	}
	
	ra1 = PG_GETARG_FLOAT8(ra_arg_pos);
	dec1 = PG_GETARG_FLOAT8(dec_arg_pos);

	if (!PG_ARGISNULL(pmra_arg_pos)&&(!PG_ARGISNULL(pmdec_arg_pos))&&
	    (!PG_ARGISNULL(epoch_arg_pos))&&(!PG_ARGISNULL(epoch2_arg_pos)))
	{
	    pmra1 = PG_GETARG_FLOAT8(pmra_arg_pos);
	    pmdec1 = PG_GETARG_FLOAT8(pmdec_arg_pos);
	    epoch1 = PG_GETARG_FLOAT8(epoch_arg_pos);
	    epoch2 = PG_GETARG_FLOAT8(epoch2_arg_pos);
	}
	else
	{
		pm_enabled = false;
		pmra1 = 0;
		pmdec1 = 0;
		epoch1 = 0;
		epoch2 = 0;
	}
	cosdec = PG_GETARG_BOOL(cosdec_arg_pos);
	ra2 = PG_GETARG_FLOAT8(ra2_arg_pos);
	dec2 = PG_GETARG_FLOAT8(dec2_arg_pos);


    if (pm_enabled)
	{
	  if (cosdec)
	    {
	      cdec = cos(dec1*Q3C_DEGRA);
	    }
	  else
	    {
	      cdec = 1;
	    }
		ra1_shift = ra1 + pmra1 * (epoch2 - epoch1) / cdec / 3600000;
		dec1_shift = dec1 + pmdec1 * (epoch2 - epoch1) / 3600000;
	}
	else
	{
		ra1_shift = ra1;
		dec1_shift = dec1;
	}
	res = q3c_dist(ra1_shift, dec1_shift, ra2, dec2);
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


PG_FUNCTION_INFO_V1(pgq3c_nearby_pm_it);
Datum pgq3c_nearby_pm_it(PG_FUNCTION_ARGS)
{
	q3c_ipix_t ipix_array[8];
	static q3c_ipix_t ipix_array_buf[8];
	static q3c_coord_t ra_cen_buf, dec_cen_buf, radius_buf;
	static q3c_coord_t pmra_buf, pmdec_buf, max_epoch_delta_buf;
	static int invocation;
	int i;
	extern struct q3c_prm hprm;
	q3c_circle_region circle;
	q3c_coord_t new_radius;
	q3c_coord_t ra_cen, dec_cen, pmra, pmdec;
	q3c_coord_t max_epoch_delta=0, radius=0;
	bool pm_enabled = true, cosdec;
	int iteration;
	const int ra_arg_pos=0, dec_arg_pos=1, pmra_arg_pos=2, pmdec_arg_pos=3,
	  cosdec_arg_pos=4, maxepochdelta_arg_pos=5, radius_arg_pos=6, iteration_arg_pos=7;

	if (PG_ARGISNULL(ra_arg_pos) || PG_ARGISNULL(dec_arg_pos) || PG_ARGISNULL(radius_arg_pos))
	  {
	    elog(ERROR, "Right Ascensions and raddii must be not null");
	  }

	ra_cen = PG_GETARG_FLOAT8(ra_arg_pos); // ra_cen
	dec_cen = PG_GETARG_FLOAT8(dec_arg_pos); // dec_cen

	if (!PG_ARGISNULL(pmra_arg_pos)&&(!PG_ARGISNULL(pmdec_arg_pos))&&
	    (!PG_ARGISNULL(maxepochdelta_arg_pos)))
	{
	    pmra = PG_GETARG_FLOAT8(pmra_arg_pos);
	    pmdec = PG_GETARG_FLOAT8(pmdec_arg_pos);
	    max_epoch_delta = PG_GETARG_FLOAT8(maxepochdelta_arg_pos);
	}
	else
	{
		pm_enabled = false;
		pmra = 0;
		pmdec = 0;
		max_epoch_delta = 0;
	}

	cosdec = PG_GETARG_BOOL(cosdec_arg_pos);
	radius = PG_GETARG_FLOAT8(radius_arg_pos); // error radius

	iteration = PG_GETARG_INT32(iteration_arg_pos); // iteration

	if ( (!isfinite(ra_cen)) || (!isfinite(dec_cen)) )
	{
		elog(ERROR, "The values of ra,dec are infinites or NaNs");
	}
	if ( (!isfinite(pmra)) || (!isfinite(pmdec)) ||
		(!isfinite(max_epoch_delta)) )
	{
		pmra =  0;
		pmdec = 0;
		max_epoch_delta = 0;
	}
	if (max_epoch_delta<0)
	{
		elog(ERROR, "The maximum epoch difference must be >=0 ");
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
	  	(radius == radius_buf) && (pmra == pmra_buf) &&
		(pmdec == pmdec_buf) && (max_epoch_delta == max_epoch_delta_buf))
		{
			PG_RETURN_INT64(ipix_array_buf[iteration]);
		}
	}

	if (pm_enabled)
	  {
	    q3c_coord_t pmra1;
	    if (cosdec) { pmra1 = pmra ; } else { pmra1 = pmra * cos(Q3C_DEGRA*dec_cen);}
	    new_radius = q3c_sqrt(pmra1 * pmra1 + pmdec * pmdec)/ 3600000 * max_epoch_delta + radius;
	  }
	else
	  {
	    new_radius = radius;
	  }
	ra_cen = UNWRAP_RA(ra_cen);
	if (q3c_fabs(dec_cen)>90) {dec_cen = q3c_fmod(dec_cen,90);}
	circle.ra = ra_cen;
	circle.dec = dec_cen;
	circle.rad = new_radius;

	q3c_get_nearby(&hprm, Q3C_CIRCLE, &circle, ipix_array);

	for(i = 0; i < 8; i++)
	{
		ipix_array_buf[i] = ipix_array[i];
	}

	ra_cen_buf = ra_cen;
	dec_cen_buf = dec_cen;
	radius_buf = radius;
	max_epoch_delta_buf = max_epoch_delta;
	pmra_buf = pmra;
	pmdec_buf = pmdec;
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

	static q3c_coord_t ra_cen_buf, dec_cen_buf, radius_buf;

	static q3c_ipix_t partials[2 * Q3C_NPARTIALS];
	static q3c_ipix_t fulls[2 * Q3C_NFULLS];

	/*  !!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!
	 * Here the Q3C_NPARTIALS and Q3C_NFULLS is the number of pairs !!! of ranges
	 * So we should have the array with the size twice bigger
	 */
	static int invocation;

	ra_cen = UNWRAP_RA(ra_cen);
	if (q3c_fabs(dec_cen)>90)
	{
		elog(ERROR, "The absolute value of declination > 90!");
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

	q3c_radial_query(&hprm, ra_cen, dec_cen, radius, fulls, partials);

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

	static q3c_coord_t ra_cen_buf, dec_cen_buf, radius_buf;
	static q3c_ipix_t partials[2 * Q3C_NPARTIALS];
	static q3c_ipix_t fulls[2 * Q3C_NFULLS];
	/*  !!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!
	 * Here the Q3C_NPARTIALS and Q3C_NFULLS is the number of pairs !!! of ranges
	 * So we should have the array with the size twice bigger
	 */

	static int invocation;

	ra_cen = UNWRAP_RA(ra_cen);
	if (q3c_fabs(dec_cen)>90)
	{
		elog(ERROR, "The absolute value of declination > 90!");
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

static q3c_coord_t read_from_array(char **p, bits8 *bitmap, int *bitmask, bool typbyval,
	char typalign, int16 typlen)
{
	q3c_coord_t val;

	/* Taken from /pgsql/src/backend/utils/adt/arrayfuncs.c
	 * function deconstruct_array
	*/
	if (bitmap && (*bitmap & *bitmask) == 0)
	{
		ereport(ERROR,
		(errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
		errmsg("null array element not allowed in this context")));
	}
	val = DatumGetFloat8(fetch_att(*p, typbyval, typlen));
	*p = att_addlength_pointer(*p, typlen, PointerGetDatum(p));
	*p = (char *) att_align_nominal(*p, typalign);
	if (bitmap)
	{
		*bitmask <<= 1;
		if (*bitmask == 0x100)
		{
			bitmap++;
			*bitmask = 1;
		}
	}
	return val;
}
/* Convert the PG array in two c arrays of ra,dec */
static int convert_pgarray2poly(ArrayType *poly_arr, q3c_coord_t *in_ra, q3c_coord_t *in_dec, int *nvert)
{
	int poly_nitems = ArrayGetNItems(ARR_NDIM(poly_arr), ARR_DIMS(poly_arr));
	Oid element_type=FLOAT8OID;
	int identical = 1;
	int16 typlen;
	bool typbyval;
	char typalign;
	int i;
	q3c_coord_t ra_cur, dec_cur;
	char *p;
	bits8 *bitmap;
	int bitmask;
	get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);

	/* Taken from /pgsql/src/backend/utils/adt/arrayfuncs.c
	 * function deconstruct_array
	 */

	if (poly_nitems % 2 != 0)
	{
		 elog(ERROR, "Invalid array argument!\nThe array should contain even number of elements");
	}
	else if (poly_nitems <= 4)
	{
		elog(ERROR, "Invalid polygon! The polygon must have more than two vertices");
	}

	p = ARR_DATA_PTR(poly_arr);
	poly_nitems /= 2;
	*nvert = poly_nitems;
	identical = 1;

	bitmap = ARR_NULLBITMAP(poly_arr);
	bitmask=1;

	for (i = 0; i < poly_nitems; i++)
	{
		ra_cur = read_from_array(&p, bitmap, &bitmask, typbyval, typalign, typlen);
		dec_cur = read_from_array(&p, bitmap, &bitmask, typbyval, typalign, typlen);

		if ((in_ra[i] != ra_cur) || (in_dec[i] != dec_cur))
		{
			identical = 0;
		}
		in_ra[i] = ra_cur;
		in_dec[i] = dec_cur;
	}
	return identical;
}

/* Convert Postgresql polygon in two c arrays */
static int convert_pgpoly2poly(POLYGON *poly, q3c_coord_t *ra, q3c_coord_t *dec, int *n)
{
	int i, npts = poly->npts;
	q3c_coord_t newx, newy;
	int identical = 1;

	*n = npts;
	if (npts < 3)
	{
		elog(ERROR, "Invalid polygon! The polygon must have more than two vertices");
	}

	for(i=0;i<npts;i++)
	{
		newx = poly->p[i].x;
		newy = poly->p[i].y;
		if ((newx != ra[i]) || (newy !=dec[i]))	{identical=0;}
		ra[i] = newx;
		dec[i] = newy;
	}
	return identical;
}


typedef struct q3c_poly_info_type{
	/*  !!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!
	* Here the Q3C_NPARTIALS and Q3C_NFULLS is the number of pairs !!! of ranges
	* So we should have the array with the size twice bigger
	*/
	int ready;
	q3c_ipix_t partials[2 * Q3C_NPARTIALS]; /* array of ipixes partially covered */
	q3c_ipix_t fulls[2 * Q3C_NFULLS]; /* array of ipixes fully covered */
	q3c_coord_t ra[Q3C_MAX_N_POLY_VERTEX],
		dec[Q3C_MAX_N_POLY_VERTEX], x[Q3C_MAX_N_POLY_VERTEX],y[Q3C_MAX_N_POLY_VERTEX],
		ax[Q3C_MAX_N_POLY_VERTEX], ay[Q3C_MAX_N_POLY_VERTEX];
	q3c_coord_t xpj[3][Q3C_MAX_N_POLY_VERTEX], ypj[3][Q3C_MAX_N_POLY_VERTEX],
			axpj[3][Q3C_MAX_N_POLY_VERTEX], aypj[3][Q3C_MAX_N_POLY_VERTEX];
		// arrays storing the ra,dec ,projected x,y
	char faces[6];
	char multi_flag;
	/* IF YOU MAKE CHANGES MAKE SURE YOU CHANGE THE COPY() FUNCTION */
} q3c_poly_info_type;

static void copy_q3c_poly_info_type(q3c_poly_info_type *a, q3c_poly_info_type *b)
{
	int i,j;
	for (i=0; i<(2* Q3C_NPARTIALS); i++)
	{
		b->partials[i]=a->partials[i];
	}
	for (i=0; i<(2* Q3C_NPARTIALS); i++)
	{
		b->fulls[i]=a->fulls[i];
	}
	for (i=0;i<Q3C_MAX_N_POLY_VERTEX;i++)
	{
		b->ra[i] = a->ra[i];
		b->dec[i] = a->dec[i];
		b->x[i] = a->x[i];
		b->y[i] = a->y[i];
		b->ax[i] = a->ax[i];
		b->ay[i] = a->ay[i];
		for (j=0;j<3;j++)
		{
			b->axpj[j][i]=a->axpj[j][i];
			b->aypj[j][i]=a->aypj[j][i];
			b->xpj[j][i]=a->xpj[j][i];
			b->ypj[j][i]=a->ypj[j][i];

		}

	}
	for (i=0;i<6;i++)
	{
		b->faces[i] = a->faces[i];
	}
	b->multi_flag = a->multi_flag;
}


/* Cache logic here is the following
when the function is called for the first time with iteration =0
I compute everything allocate memory and store computations in the static variable
and locally allocated q3c_poly_info_table
when the function is called for the first time and iteration  !=0
I allocate new memory, copy stuff from static variable into locally allocated stuff
I make no checks of the data
If the function is called for the second time (i.e. fn_extra is not null)
I blindly assume everything is EXACTLY the same and do not recompute anything
as the q3c_poly_query_it() is the internal function and is ONLY supposed
to be called with the constant polygon
*/
PG_FUNCTION_INFO_V1(pgq3c_poly_query_it);
Datum pgq3c_poly_query_it(PG_FUNCTION_ARGS)
{
	ArrayType *poly_arr = PG_GETARG_ARRAYTYPE_P(0);
	int iteration = PG_GETARG_INT32(1); /* iteration */
	int full_flag = PG_GETARG_INT32(2); /* full_flag */
	/* 1 means full, 0 means partial*/
	extern struct q3c_prm hprm;

	char too_large = 0;
	q3c_poly_info_type *qpit;
	q3c_poly qp;
	static int good_cache;
	int first_call;
	int identical=0;
	static q3c_poly_info_type lqpit;

	if (fcinfo->flinfo->fn_extra==0)
	{
		// allocate memory where we are going to store converted info
		fcinfo->flinfo->fn_extra = MemoryContextAlloc(fcinfo->flinfo->fn_mcxt, sizeof(q3c_poly_info_type));
		first_call=1;
	}
	else
	{
		first_call =0;
	}

	qpit = (q3c_poly_info_type*) (fcinfo->flinfo->fn_extra);

	/* if second call it is easy */
	if (!first_call)
	{
		if (full_flag)
		{
			PG_RETURN_INT64(qpit->fulls[iteration]);
		}
		else
		{
			PG_RETURN_INT64(qpit->partials[iteration]);
		}
	}

	if (iteration > 0)
	{
		copy_q3c_poly_info_type(&lqpit, qpit);
	}
	qp.ra = qpit->ra;
	qp.dec = qpit->dec;
	qp.x = qpit->x;
	qp.y = qpit->y;
	qp.ax = qpit->ax;
	qp.ay = qpit->ay;

	identical = convert_pgarray2poly(poly_arr, qp.ra, qp.dec, &(qp.n));
	/* We fill the arrays and check if it matches what we had before */

	if (!identical || !good_cache)
	{
		q3c_poly_query(&hprm, &qp, qpit->fulls, qpit->partials, &too_large);
		if (too_large)
		{
			elog(ERROR, "The polygon is too large. Polygons having diameter >~23 degrees are unsupported");
		}
	}
	if (iteration==0)
	{
		good_cache=0;
		copy_q3c_poly_info_type(qpit, &lqpit);
		good_cache=1;
	}

	if (full_flag)
	{
		PG_RETURN_INT64(qpit->fulls[iteration]);
	}
	else
	{
		PG_RETURN_INT64(qpit->partials[iteration]);
	}
}


PG_FUNCTION_INFO_V1(pgq3c_poly_query1_it);
Datum pgq3c_poly_query1_it(PG_FUNCTION_ARGS)
{
	POLYGON *poly_arr = PG_GETARG_POLYGON_P(0);
	int iteration = PG_GETARG_INT32(1); /* iteration */
	int full_flag = PG_GETARG_INT32(2); /* full_flag */
	/* 1 means full, 0 means partial*/
	extern struct q3c_prm hprm;
	char too_large = 0;
	q3c_poly_info_type *qpit;
	q3c_poly qp;
	static int good_cache;
	int first_call;
	int identical=0;
	static q3c_poly_info_type lqpit;

	if (fcinfo->flinfo->fn_extra==0)
	{
		// allocate memory where we are going to store converted info
		fcinfo->flinfo->fn_extra = MemoryContextAlloc(fcinfo->flinfo->fn_mcxt, sizeof(q3c_poly_info_type));
		first_call=1;
	}
	else
	{
		first_call =0;
	}

	qpit = (q3c_poly_info_type*) (fcinfo->flinfo->fn_extra);

	/* if second call it is easy */
	if (!first_call)
	{
		if (full_flag)
		{
			PG_RETURN_INT64(qpit->fulls[iteration]);
		}
		else
		{
			PG_RETURN_INT64(qpit->partials[iteration]);
		}
	}

	if (iteration > 0)
	{
		copy_q3c_poly_info_type(&lqpit, qpit);
	}
	qp.ra = qpit->ra;
	qp.dec = qpit->dec;
	qp.x = qpit->x;
	qp.y = qpit->y;
	qp.ax = qpit->ax;
	qp.ay = qpit->ay;

	identical = convert_pgpoly2poly(poly_arr, qp.ra, qp.dec, &(qp.n));
	/* We fill the arrays and check if it matches what we had before */

	if (!identical || !good_cache)
	{
		q3c_poly_query(&hprm, &qp, qpit->fulls, qpit->partials, &too_large);
		if (too_large)
		{
			elog(ERROR, "The polygon is too large. Polygons having diameter >~23 degrees are unsupported");
		}
	}
	if (iteration==0)
	{
		good_cache=0;
		copy_q3c_poly_info_type(qpit, &lqpit);
		good_cache=1;
	}

	if (full_flag)
	{
		PG_RETURN_INT64(qpit->fulls[iteration]);
	}
	else
	{
		PG_RETURN_INT64(qpit->partials[iteration]);
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


/* The caching assumptions for this code are
   For the following calls of the function, if the polygon is
   exactly the same in the I will not reproject
 */
PG_FUNCTION_INFO_V1(pgq3c_in_poly);
Datum pgq3c_in_poly(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(0); // ra_cen
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(1); // dec_cen
	ArrayType *poly_arr = PG_GETARG_ARRAYTYPE_P(2); // ra_cen
	extern struct q3c_prm hprm;
	char too_large = 0;
	int nvert;
	bool result;
	int identical;
	q3c_poly_info_type *qpit;

	if (fcinfo->flinfo->fn_extra==0)
	{
		// allocate memory where we are going to store converted info
		fcinfo->flinfo->fn_extra = MemoryContextAlloc(fcinfo->flinfo->fn_mcxt, sizeof(q3c_poly_info_type));
		((q3c_poly_info_type*) (fcinfo->flinfo->fn_extra))->ready = 0;
	}

	qpit = (q3c_poly_info_type*) (fcinfo->flinfo->fn_extra);

	identical = convert_pgarray2poly(poly_arr, qpit->ra, qpit->dec, &nvert) && qpit->ready;

	result = q3c_check_sphere_point_in_poly(&hprm, nvert, qpit->ra, qpit->dec,
											ra_cen, dec_cen, &too_large, identical,
											qpit->xpj, qpit->ypj,
										  qpit->axpj, qpit->aypj,
										  qpit->faces, &(qpit->multi_flag)
										) !=	Q3C_DISJUNCT;
	if (too_large)
	{
		elog(ERROR, "The polygon is too large. Polygons having diameter >~23 degrees are unsupported");
	}
	qpit->ready = 1;

	PG_RETURN_BOOL(result);
}


PG_FUNCTION_INFO_V1(pgq3c_in_poly1);
Datum pgq3c_in_poly1(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(0); // ra_cen
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(1); // dec_cen
	POLYGON *poly = PG_GETARG_POLYGON_P(2); // ra_cen
	extern struct q3c_prm hprm;
	char too_large = 0;
	int nvert;
	bool result;
	int identical;
	q3c_poly_info_type *qpit;

	if (fcinfo->flinfo->fn_extra==0)
	{
		// allocate memory where we are going to store converted info
		fcinfo->flinfo->fn_extra = MemoryContextAlloc(fcinfo->flinfo->fn_mcxt, sizeof(q3c_poly_info_type));
		((q3c_poly_info_type*) (fcinfo->flinfo->fn_extra))->ready = 0;
	}

	qpit = (q3c_poly_info_type*) (fcinfo->flinfo->fn_extra);

	identical = convert_pgpoly2poly(poly, qpit->ra, qpit->dec, &nvert) && qpit->ready;

	result = q3c_check_sphere_point_in_poly(&hprm, nvert, qpit->ra, qpit->dec,
											ra_cen, dec_cen, &too_large, identical,
											qpit->xpj, qpit->ypj,
										  qpit->axpj, qpit->aypj,
										  qpit->faces, &(qpit->multi_flag)
										) !=	Q3C_DISJUNCT;
	qpit->ready = 1;
	if (too_large)
	{
		elog(ERROR, "The polygon is too large. Polygons having diameter >~23 degrees are unsupported");
	}

	PG_RETURN_BOOL(result);
}
