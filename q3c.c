/*
       Copyright (C) 2004-2023 Sergey Koposov

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


#include <string.h>

/* Postgres stuff */
#include "postgres.h"
#include "access/stratnum.h"
#include "catalog/namespace.h"
#include "catalog/pg_opfamily_d.h"
#include "executor/spi.h"
#include "nodes/makefuncs.h"
#include "nodes/nodeFuncs.h"
#include "nodes/supportnodes.h"
#include "optimizer/clauses.h"
#include "parser/parse_func.h"
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
Datum pgq3c_radial_query(PG_FUNCTION_ARGS);
Datum pgq3c_radial_query_real(PG_FUNCTION_ARGS);
Datum pgq3c_radial_query_exact(PG_FUNCTION_ARGS);
Datum pgq3c_radial_query_exact_real(PG_FUNCTION_ARGS);
Datum pgq3c_radial_query_support(PG_FUNCTION_ARGS);
Datum pgq3c_ellipse_query_it(PG_FUNCTION_ARGS);
Datum pgq3c_ellipse_query(PG_FUNCTION_ARGS);
Datum pgq3c_ellipse_query_support(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query_array(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query_array_real(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query_polygon(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query_polygon_real(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query_support(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query_it(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query1_it(PG_FUNCTION_ARGS);
Datum pgq3c_in_ellipse(PG_FUNCTION_ARGS);
Datum pgq3c_in_poly(PG_FUNCTION_ARGS);
Datum pgq3c_in_poly1(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query_exact_array(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query_exact_array_real(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query_exact_polygon(PG_FUNCTION_ARGS);
Datum pgq3c_poly_query_exact_polygon_real(PG_FUNCTION_ARGS);

Datum pgq3c_get_version(PG_FUNCTION_ARGS);
Datum pgq3c_sel(PG_FUNCTION_ARGS);
Datum pgq3c_seljoin(PG_FUNCTION_ARGS);
Datum pgq3c_seloper(PG_FUNCTION_ARGS);

static int convert_pgarray2poly(ArrayType *poly_arr, q3c_coord_t *in_ra,
								q3c_coord_t *in_dec, int *nvert);
static int convert_pgpoly2poly(POLYGON *poly, q3c_coord_t *ra,
							   q3c_coord_t *dec, int *nvert);
static Oid q3c_lookup_function_in_namespace(Oid reference_funcid,
											const char *proname,
											int nargs,
											const Oid *argtypes);
static Oid q3c_lookup_bigint_operator(StrategyNumber strategy);
static Const *q3c_make_int8_const(int64 value);
static FuncExpr *q3c_make_ang2ipix_call(Oid funcid, Expr *ra, Expr *dec);
static void q3c_legacy_iterator_error(const char *proname);
static Expr *q3c_build_range_or_clause(Oid ang2ipix_funcid, Oid ge_opid,
									   Oid lt_opid, Expr *ra, Expr *dec,
									   const q3c_ipix_t *ranges, int nranges);
static Expr *q3c_build_ranged_query_clause(Oid funcid, Expr *ra, Expr *dec,
										   const char *exact_name,
										   const Oid *exact_argtypes,
										   int exact_nargs, List *exact_args,
										   const q3c_ipix_t *fulls,
										   const q3c_ipix_t *partials);
static Expr *q3c_build_radial_query_simplified_clause(PlannerInfo *root,
													  Oid funcid, List *args);
static Expr *q3c_build_ellipse_query_simplified_clause(PlannerInfo *root,
													   Oid funcid, List *args);
static Expr *q3c_build_poly_query_simplified_clause(PlannerInfo *root,
													Oid funcid, List *args);
static bool q3c_estimate_const_expr(PlannerInfo *root, Node *node,
									Const **value_const);
static bool q3c_estimate_float8_expr(PlannerInfo *root, Node *node,
									 double *value);
static bool q3c_estimate_poly_expr(PlannerInfo *root, Node *node, Oid polytype,
								   q3c_coord_t *ra, q3c_coord_t *dec,
								   int *nvert);
static bool q3c_poly_query_array_match(FunctionCallInfo fcinfo,
									   q3c_coord_t ra_cen,
									   q3c_coord_t dec_cen,
									   ArrayType *poly_arr);
static bool q3c_poly_query_polygon_match(FunctionCallInfo fcinfo,
										 q3c_coord_t ra_cen,
										 q3c_coord_t dec_cen,
										 POLYGON *poly);
static bool q3c_radial_query_match(q3c_coord_t ra, q3c_coord_t dec,
								   q3c_coord_t ra_cen,
								   q3c_coord_t dec_cen,
								   q3c_coord_t radius);
static bool q3c_ellipse_query_match(q3c_coord_t ra, q3c_coord_t dec,
									q3c_coord_t ra_cen,
									q3c_coord_t dec_cen,
									q3c_coord_t semimajax,
									q3c_coord_t axis_ratio,
									q3c_coord_t PA);


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
	ratio = 3.14 * rad * rad / 41252.;  /* pi*r^2/whole_sky_area */

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
	ratio = 3.14 * rad * rad / 41252.;  /* pi*r^2/whole_sky_area */

	/* clamp at 0, 1*/
	CLAMP_PROBABILITY(ratio);

	PG_RETURN_FLOAT8(ratio);
}


static Oid
q3c_lookup_function_in_namespace(Oid reference_funcid, const char *proname,
								 int nargs, const Oid *argtypes)
{
	Oid nspid;
	Oid funcid;
	char *nspname;
	List *qualified_name;

	nspid = get_func_namespace(reference_funcid);
	if (!OidIsValid(nspid))
	{
		elog(ERROR, "could not resolve namespace for function %u", reference_funcid);
	}

	nspname = get_namespace_name(nspid);
	if (nspname == NULL)
	{
		elog(ERROR, "could not resolve namespace name for namespace %u", nspid);
	}

	qualified_name = list_make2(makeString(nspname),
								makeString(pstrdup(proname)));
	funcid = LookupFuncName(qualified_name, nargs, argtypes, false);

	if (!OidIsValid(funcid))
	{
		elog(ERROR, "could not resolve function %s in namespace %s",
			 proname, nspname);
	}

	return funcid;
}


static Oid
q3c_lookup_bigint_operator(StrategyNumber strategy)
{
	Oid opid;

	opid = get_opfamily_member(INTEGER_BTREE_FAM_OID, INT8OID, INT8OID,
							   strategy);
	if (!OidIsValid(opid))
	{
		elog(ERROR, "missing bigint operator for btree strategy %d",
			 (int) strategy);
	}

	return opid;
}


static Const *
q3c_make_int8_const(int64 value)
{
	static int16 typlen = 0;
	static bool typbyval = false;
	static char typalign = 0;

	if (typlen == 0)
	{
		get_typlenbyvalalign(INT8OID, &typlen, &typbyval, &typalign);
	}

	return makeConst(INT8OID, -1, InvalidOid, typlen,
					 Int64GetDatum(value), false, typbyval);
}


static FuncExpr *
q3c_make_ang2ipix_call(Oid funcid, Expr *ra, Expr *dec)
{
	List *args;

	args = list_make2(copyObject(ra), copyObject(dec));
	return makeFuncExpr(funcid, INT8OID, args, InvalidOid, InvalidOid,
						COERCE_EXPLICIT_CALL);
}


static Expr *
q3c_build_range_or_clause(Oid ang2ipix_funcid, Oid ge_opid, Oid lt_opid,
						  Expr *ra, Expr *dec, const q3c_ipix_t *ranges,
						  int nranges)
{
	List *or_clauses = NIL;
	int i;

	for (i = 0; i < nranges; i += 2)
	{
		Expr *lower_cmp;
		Expr *upper_cmp;

		if (ranges[i] == 1 && ranges[i + 1] == -1)
		{
			break;
		}

		lower_cmp = make_opclause(ge_opid, BOOLOID, false,
								  (Expr *) q3c_make_ang2ipix_call(ang2ipix_funcid,
																  ra, dec),
								  (Expr *) q3c_make_int8_const(ranges[i]),
								  InvalidOid, InvalidOid);
		upper_cmp = make_opclause(lt_opid, BOOLOID, false,
								  (Expr *) q3c_make_ang2ipix_call(ang2ipix_funcid,
																  ra, dec),
								  (Expr *) q3c_make_int8_const(ranges[i + 1]),
								  InvalidOid, InvalidOid);
		or_clauses = lappend(or_clauses,
							 make_andclause(list_make2(lower_cmp, upper_cmp)));
	}

	if (or_clauses == NIL)
	{
		return NULL;
	}

	return make_orclause(or_clauses);
}


static Expr *
q3c_build_ranged_query_clause(Oid funcid, Expr *ra, Expr *dec,
							  const char *exact_name,
							  const Oid *exact_argtypes,
							  int exact_nargs, List *exact_args,
							  const q3c_ipix_t *fulls,
							  const q3c_ipix_t *partials)
{
	Oid ang2ipix_argtypes[2];
	Oid ang2ipix_funcid;
	Oid exact_funcid;
	Oid ge_opid;
	Oid lt_opid;
	Expr *full_clause;
	Expr *partial_clause;
	Expr *exact_clause;
	List *range_clauses = NIL;
	Expr *range_clause;

	ang2ipix_argtypes[0] = exprType((Node *) ra);
	ang2ipix_argtypes[1] = exprType((Node *) dec);

	ang2ipix_funcid = q3c_lookup_function_in_namespace(funcid, "q3c_ang2ipix",
													   2, ang2ipix_argtypes);
	exact_funcid =
		q3c_lookup_function_in_namespace(funcid, exact_name, exact_nargs,
										 exact_argtypes);
	ge_opid = q3c_lookup_bigint_operator(BTGreaterEqualStrategyNumber);
	lt_opid = q3c_lookup_bigint_operator(BTLessStrategyNumber);

	full_clause = q3c_build_range_or_clause(ang2ipix_funcid, ge_opid, lt_opid,
											ra, dec, fulls,
											2 * Q3C_NFULLS);
	partial_clause = q3c_build_range_or_clause(ang2ipix_funcid, ge_opid, lt_opid,
											   ra, dec, partials,
											   2 * Q3C_NPARTIALS);

	if (full_clause != NULL)
	{
		range_clauses = lappend(range_clauses, full_clause);
	}
	if (partial_clause != NULL)
	{
		range_clauses = lappend(range_clauses, partial_clause);
	}

	exact_clause = (Expr *) makeFuncExpr(exact_funcid, BOOLOID,
										 copyObject(exact_args),
										 InvalidOid, InvalidOid,
										 COERCE_EXPLICIT_CALL);

	if (range_clauses == NIL)
	{
		return exact_clause;
	}

	if (list_length(range_clauses) == 1)
	{
		range_clause = (Expr *) linitial(range_clauses);
	}
	else
	{
		range_clause = make_orclause(range_clauses);
	}

	return make_andclause(list_make2(range_clause, exact_clause));
}


static Expr *
q3c_build_radial_query_simplified_clause(PlannerInfo *root, Oid funcid,
										 List *args)
{
	extern struct q3c_prm hprm;
	Oid exact_argtypes[5];
	Expr *ra;
	Expr *dec;
	q3c_coord_t ra_cen_value;
	q3c_coord_t dec_cen_value;
	q3c_coord_t radius_value;
	q3c_ipix_t fulls[2 * Q3C_NFULLS];
	q3c_ipix_t partials[2 * Q3C_NPARTIALS];

	if (list_length(args) != 5)
	{
		return NULL;
	}

	ra = (Expr *) linitial(args);
	dec = (Expr *) lsecond(args);

	exact_argtypes[0] = exprType((Node *) ra);
	exact_argtypes[1] = exprType((Node *) dec);
	exact_argtypes[2] = FLOAT8OID;
	exact_argtypes[3] = FLOAT8OID;
	exact_argtypes[4] = FLOAT8OID;

	if (!q3c_estimate_float8_expr(root, (Node *) lthird(args), &ra_cen_value) ||
		!q3c_estimate_float8_expr(root, (Node *) lfourth(args), &dec_cen_value) ||
		!q3c_estimate_float8_expr(root, (Node *) llast(args), &radius_value))
	{
		return NULL;
	}

	q3c_radial_query(&hprm, ra_cen_value, dec_cen_value, radius_value,
					 fulls, partials);

	return q3c_build_ranged_query_clause(funcid, ra, dec,
										 "q3c_radial_query_exact",
										 exact_argtypes, 5, args,
										 fulls, partials);
}


static Expr *
q3c_build_ellipse_query_simplified_clause(PlannerInfo *root, Oid funcid,
										  List *args)
{
	extern struct q3c_prm hprm;
	Oid exact_argtypes[7];
	Expr *ra;
	Expr *dec;
	double ra_cen_value;
	double dec_cen_value;
	double semimajax_value;
	double axis_ratio_value;
	double PA_value;
	double ell_value;
	q3c_ipix_t fulls[2 * Q3C_NFULLS];
	q3c_ipix_t partials[2 * Q3C_NPARTIALS];

	if (list_length(args) != 7)
	{
		return NULL;
	}

	ra = (Expr *) linitial(args);
	dec = (Expr *) lsecond(args);

	exact_argtypes[0] = exprType((Node *) ra);
	exact_argtypes[1] = exprType((Node *) dec);
	exact_argtypes[2] = FLOAT8OID;
	exact_argtypes[3] = FLOAT8OID;
	exact_argtypes[4] = FLOAT8OID;
	exact_argtypes[5] = FLOAT8OID;
	exact_argtypes[6] = FLOAT8OID;

	if (!q3c_estimate_float8_expr(root, (Node *) lthird(args), &ra_cen_value) ||
		!q3c_estimate_float8_expr(root, (Node *) lfourth(args), &dec_cen_value) ||
		!q3c_estimate_float8_expr(root, (Node *) list_nth(args, 4), &semimajax_value) ||
		!q3c_estimate_float8_expr(root, (Node *) list_nth(args, 5), &axis_ratio_value) ||
		!q3c_estimate_float8_expr(root, (Node *) list_nth(args, 6), &PA_value))
	{
		return NULL;
	}

	ell_value = q3c_sqrt(1 - axis_ratio_value * axis_ratio_value);
	q3c_ellipse_query(&hprm, ra_cen_value, dec_cen_value, semimajax_value,
					  ell_value, PA_value, fulls, partials);

	return q3c_build_ranged_query_clause(funcid, ra, dec,
										 "q3c_in_ellipse",
										 exact_argtypes, 7, args,
										 fulls, partials);
}


static bool
q3c_estimate_const_expr(PlannerInfo *root, Node *node, Const **value_const)
{
	Node *estimated;

	if (value_const == NULL || node == NULL)
	{
		return false;
	}

	estimated = node;
	if (root != NULL)
	{
		estimated = estimate_expression_value(root, node);
	}

	if (estimated == NULL || !IsA(estimated, Const))
	{
		return false;
	}

	*value_const = (Const *) estimated;
	return !(*value_const)->constisnull;
}


static void
q3c_legacy_iterator_error(const char *proname)
{
	ereport(ERROR,
			(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
			 errmsg("%s() is only kept as a compatibility stub for old extension versions", proname)));
}


static Expr *
q3c_build_poly_query_simplified_clause(PlannerInfo *root, Oid funcid,
									   List *args)
{
	extern struct q3c_prm hprm;
	Oid exact_argtypes[3];
	Oid polytype;
	Expr *ra;
	Expr *dec;
	Expr *poly_expr;
	q3c_coord_t poly_ra[Q3C_MAX_N_POLY_VERTEX];
	q3c_coord_t poly_dec[Q3C_MAX_N_POLY_VERTEX];
	q3c_coord_t poly_x[Q3C_MAX_N_POLY_VERTEX];
	q3c_coord_t poly_y[Q3C_MAX_N_POLY_VERTEX];
	q3c_coord_t poly_ax[Q3C_MAX_N_POLY_VERTEX];
	q3c_coord_t poly_ay[Q3C_MAX_N_POLY_VERTEX];
	q3c_poly qp;
	q3c_ipix_t fulls[2 * Q3C_NFULLS];
	q3c_ipix_t partials[2 * Q3C_NPARTIALS];
	int nvert;
	char too_large = 0;

	if (list_length(args) != 3)
	{
		return NULL;
	}

	ra = (Expr *) linitial(args);
	dec = (Expr *) lsecond(args);
	poly_expr = (Expr *) llast(args);
	polytype = exprType((Node *) poly_expr);

	exact_argtypes[0] = exprType((Node *) ra);
	exact_argtypes[1] = exprType((Node *) dec);
	exact_argtypes[2] = polytype;

	if (!q3c_estimate_poly_expr(root, (Node *) poly_expr, polytype,
								poly_ra, poly_dec, &nvert))
	{
		return NULL;
	}

	qp.n = nvert;
	qp.ra = poly_ra;
	qp.dec = poly_dec;
	qp.x = poly_x;
	qp.y = poly_y;
	qp.ax = poly_ax;
	qp.ay = poly_ay;

	q3c_poly_query(&hprm, &qp, fulls, partials, &too_large);
	if (too_large)
	{
		elog(ERROR, "The polygon is too large. Polygons having diameter >~23 degrees are unsupported");
	}

	return q3c_build_ranged_query_clause(funcid, ra, dec,
										 "q3c_in_poly",
										 exact_argtypes, 3, args,
										 fulls, partials);
}


static bool
q3c_estimate_float8_expr(PlannerInfo *root, Node *node, double *value)
{
	Const *value_const;

	if (value == NULL || !q3c_estimate_const_expr(root, node, &value_const))
	{
		return false;
	}

	if (value_const->consttype != FLOAT8OID)
	{
		return false;
	}

	*value = DatumGetFloat8(value_const->constvalue);
	return isfinite(*value);
}


static bool
q3c_estimate_poly_expr(PlannerInfo *root, Node *node, Oid polytype,
					   q3c_coord_t *ra, q3c_coord_t *dec, int *nvert)
{
	Const *value_const;

	if (ra == NULL || dec == NULL || nvert == NULL ||
		!q3c_estimate_const_expr(root, node, &value_const))
	{
		return false;
	}

	if (value_const->consttype != polytype)
	{
		return false;
	}

	if (polytype == get_array_type(FLOAT8OID))
	{
		convert_pgarray2poly(DatumGetArrayTypeP(value_const->constvalue),
							 ra, dec, nvert);
		return true;
	}

	if (polytype == POLYGONOID)
	{
		convert_pgpoly2poly(DatumGetPolygonP(value_const->constvalue),
							ra, dec, nvert);
		return true;
	}

	return false;
}


static bool
q3c_radial_query_match(q3c_coord_t ra, q3c_coord_t dec, q3c_coord_t ra_cen,
					   q3c_coord_t dec_cen, q3c_coord_t radius)
{
	q3c_coord_t threshold;
	q3c_coord_t distance;

	if ((!isfinite(ra)) || (!isfinite(dec)) ||
		(!isfinite(ra_cen)) || (!isfinite(dec_cen)) ||
		(!isfinite(radius)))
	{
		return false;
	}

	ra_cen = UNWRAP_RA(ra_cen);
	if (q3c_fabs(dec_cen) > 90)
	{
		elog(ERROR, "The absolute value of declination > 90!");
	}

	threshold = sin(radius * Q3C_DEGRA / 2);
	threshold *= threshold;
	distance = q3c_sindist(ra, dec, ra_cen, dec_cen);

	return distance < threshold;
}


static bool
q3c_ellipse_query_match(q3c_coord_t ra, q3c_coord_t dec, q3c_coord_t ra_cen,
						q3c_coord_t dec_cen, q3c_coord_t semimajax,
						q3c_coord_t axis_ratio, q3c_coord_t PA)
{
	q3c_coord_t e;

	if ((!isfinite(ra)) || (!isfinite(dec)) ||
		(!isfinite(ra_cen)) || (!isfinite(dec_cen)) ||
		(!isfinite(semimajax)) || (!isfinite(axis_ratio)) ||
		(!isfinite(PA)))
	{
		return false;
	}

	ra_cen = UNWRAP_RA(ra_cen);
	if (q3c_fabs(dec_cen) > 90)
	{
		elog(ERROR, "The absolute value of declination > 90!");
	}

	e = q3c_sqrt(1 - axis_ratio * axis_ratio);
	return q3c_in_ellipse(ra_cen, dec_cen, ra, dec, semimajax, e, PA);
}


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

	if (invocation == 0)
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
	invocation = 1;

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

	if (invocation == 0)
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
	invocation = 1;

	PG_RETURN_INT64(ipix);
}

PG_FUNCTION_INFO_V1(pgq3c_ipix2ang);
Datum pgq3c_ipix2ang(PG_FUNCTION_ARGS)
{
	extern struct q3c_prm hprm;
	q3c_ipix_t ipix;
	q3c_coord_t ra, dec;

	Datum       *data;
	int16 typlen;
	bool typbyval;
	char typalign;
	ArrayType  *result;
	ipix = PG_GETARG_INT64(0);
	if ((ipix < 0) || (ipix > Q3C_MAX_IPIX))
	{
		elog(ERROR, "Invalid ipix value");

	}
	q3c_ipix2ang(&hprm, ipix, &ra, &dec);

	data = ( Datum *) palloc(sizeof(Datum) * 2);
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
	if (ipix < 0)
	{
		elog(ERROR, "Invalid ipix value");
	}
	if (ipix > Q3C_MAX_IPIX)
	{
		elog(ERROR, "Invalid ipix value");
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
	const int ra_arg_pos = 0, dec_arg_pos = 1, pmra_arg_pos = 2, pmdec_arg_pos = 3,
	          cosdec_arg_pos = 4, epoch_arg_pos = 5, ra2_arg_pos = 6, dec2_arg_pos = 7,
	          epoch2_arg_pos = 8;

	if (PG_ARGISNULL(ra_arg_pos) || PG_ARGISNULL(dec_arg_pos) ||
	    PG_ARGISNULL(ra2_arg_pos) || PG_ARGISNULL(dec2_arg_pos))
	{
		PG_RETURN_NULL();
	}

	ra1 = PG_GETARG_FLOAT8(ra_arg_pos);
	dec1 = PG_GETARG_FLOAT8(dec_arg_pos);

	if ((!PG_ARGISNULL(pmra_arg_pos)) &&
	    (!PG_ARGISNULL(pmdec_arg_pos)) &&
	    (!PG_ARGISNULL(epoch_arg_pos)) && (!PG_ARGISNULL(epoch2_arg_pos)))
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
	cosdec = PG_GETARG_INT32(cosdec_arg_pos) != 0;
	ra2 = PG_GETARG_FLOAT8(ra2_arg_pos);
	dec2 = PG_GETARG_FLOAT8(dec2_arg_pos);


	if (pm_enabled)
	{
		if (cosdec)
		{
			cdec = cos(dec1 * Q3C_DEGRA);
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
	q3c_coord_t ra1, dec1, ra2, dec2, ra1_shift, dec1_shift, cdec = 1;
	bool pm_enabled = true, cosdec;
	q3c_coord_t res;

	const int ra_arg_pos = 0, dec_arg_pos = 1, pmra_arg_pos = 2, pmdec_arg_pos = 3,
	          cosdec_arg_pos = 4, epoch_arg_pos = 5, ra2_arg_pos = 6, dec2_arg_pos = 7,
	          epoch2_arg_pos = 8;

	if (PG_ARGISNULL(ra_arg_pos) || PG_ARGISNULL(dec_arg_pos) ||
	    PG_ARGISNULL(ra2_arg_pos) || PG_ARGISNULL(dec2_arg_pos))
	{
		PG_RETURN_NULL();
	}

	ra1 = PG_GETARG_FLOAT8(ra_arg_pos);
	dec1 = PG_GETARG_FLOAT8(dec_arg_pos);

	if ((!PG_ARGISNULL(pmra_arg_pos)) && (!PG_ARGISNULL(pmdec_arg_pos)) &&
	    (!PG_ARGISNULL(epoch_arg_pos)) && (!PG_ARGISNULL(epoch2_arg_pos)))
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
	cosdec = PG_GETARG_INT32(cosdec_arg_pos) != 0;
	ra2 = PG_GETARG_FLOAT8(ra2_arg_pos);
	dec2 = PG_GETARG_FLOAT8(dec2_arg_pos);


	if (pm_enabled)
	{
		if (cosdec)
		{
			cdec = cos(dec1 * Q3C_DEGRA);
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
	if (q3c_fabs(dec_cen) > 90) {dec_cen = q3c_fmod(dec_cen,90);}
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

	invocation = 1;
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
	q3c_coord_t max_epoch_delta = 0, radius = 0;
	bool pm_enabled = true, cosdec;
	int iteration;
	const int ra_arg_pos = 0, dec_arg_pos = 1, pmra_arg_pos = 2, pmdec_arg_pos = 3,
	          cosdec_arg_pos = 4, maxepochdelta_arg_pos = 5, radius_arg_pos = 6, iteration_arg_pos = 7;

	if (PG_ARGISNULL(ra_arg_pos) || PG_ARGISNULL(dec_arg_pos) || PG_ARGISNULL(radius_arg_pos))
	{
		elog(ERROR, "Right Ascensions and raddii must be not null");
	}

	ra_cen = PG_GETARG_FLOAT8(ra_arg_pos); // ra_cen
	dec_cen = PG_GETARG_FLOAT8(dec_arg_pos); // dec_cen

	if ((!PG_ARGISNULL(pmra_arg_pos)) && (!PG_ARGISNULL(pmdec_arg_pos)) &&
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

	cosdec = PG_GETARG_INT32(cosdec_arg_pos) != 0;
	radius = PG_GETARG_FLOAT8(radius_arg_pos); // error radius

	iteration = PG_GETARG_INT32(iteration_arg_pos); // iteration

	if ( (!isfinite(ra_cen)) || (!isfinite(dec_cen)) )
	{
		elog(ERROR, "The values of ra,dec are infinites or NaNs");
	}
	if ( (!isfinite(pmra)) || (!isfinite(pmdec)) ||
	     (!isfinite(max_epoch_delta)) )
	{
		pmra = 0;
		pmdec = 0;
		max_epoch_delta = 0;
	}
	if (max_epoch_delta < 0)
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
		if (cosdec) { pmra1 = pmra; } else { pmra1 = pmra * cos(Q3C_DEGRA * dec_cen);}
		new_radius = q3c_sqrt(pmra1 * pmra1 + pmdec * pmdec) / 3600000 * max_epoch_delta + radius;
	}
	else
	{
		new_radius = radius;
	}
	ra_cen = UNWRAP_RA(ra_cen);
	if (q3c_fabs(dec_cen) > 90) {dec_cen = q3c_fmod(dec_cen,90);}
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
	invocation = 1;
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
	if (q3c_fabs(dec_cen) > 90) {dec_cen = q3c_fmod(dec_cen,90);}

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

	invocation = 1;
	PG_RETURN_INT64(ipix_array_buf[iteration]);
}






PG_FUNCTION_INFO_V1(pgq3c_radial_query_it);
Datum pgq3c_radial_query_it(PG_FUNCTION_ARGS)
{
	q3c_legacy_iterator_error("q3c_radial_query_it");
	PG_RETURN_NULL();
}


PG_FUNCTION_INFO_V1(pgq3c_radial_query_exact);
Datum pgq3c_radial_query_exact(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra = PG_GETARG_FLOAT8(0);
	q3c_coord_t dec = PG_GETARG_FLOAT8(1);
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(2);
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(3);
	q3c_coord_t radius = PG_GETARG_FLOAT8(4);

	PG_RETURN_BOOL(q3c_radial_query_match(ra, dec, ra_cen, dec_cen, radius));
}


PG_FUNCTION_INFO_V1(pgq3c_radial_query_exact_real);
Datum pgq3c_radial_query_exact_real(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra = PG_GETARG_FLOAT4(0);
	q3c_coord_t dec = PG_GETARG_FLOAT4(1);
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(2);
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(3);
	q3c_coord_t radius = PG_GETARG_FLOAT8(4);

	PG_RETURN_BOOL(q3c_radial_query_match(ra, dec, ra_cen, dec_cen, radius));
}


PG_FUNCTION_INFO_V1(pgq3c_radial_query);
Datum pgq3c_radial_query(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra = PG_GETARG_FLOAT8(0);
	q3c_coord_t dec = PG_GETARG_FLOAT8(1);
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(2);
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(3);
	q3c_coord_t radius = PG_GETARG_FLOAT8(4);

	PG_RETURN_BOOL(q3c_radial_query_match(ra, dec, ra_cen, dec_cen, radius));
}


PG_FUNCTION_INFO_V1(pgq3c_radial_query_real);
Datum pgq3c_radial_query_real(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra = PG_GETARG_FLOAT4(0);
	q3c_coord_t dec = PG_GETARG_FLOAT4(1);
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(2);
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(3);
	q3c_coord_t radius = PG_GETARG_FLOAT8(4);

	PG_RETURN_BOOL(q3c_radial_query_match(ra, dec, ra_cen, dec_cen, radius));
}


PG_FUNCTION_INFO_V1(pgq3c_radial_query_support);
Datum pgq3c_radial_query_support(PG_FUNCTION_ARGS)
{
	Node *rawreq = (Node *) PG_GETARG_POINTER(0);
	Node *ret = NULL;

	if (IsA(rawreq, SupportRequestSimplify))
	{
		SupportRequestSimplify *req = (SupportRequestSimplify *) rawreq;

		ret = (Node *) q3c_build_radial_query_simplified_clause(
			req->root, req->fcall->funcid, req->fcall->args);
	}

	PG_RETURN_POINTER(ret);
}


PG_FUNCTION_INFO_V1(pgq3c_ellipse_query);
Datum pgq3c_ellipse_query(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra = PG_GETARG_FLOAT8(0);
	q3c_coord_t dec = PG_GETARG_FLOAT8(1);
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(2);
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(3);
	q3c_coord_t semimajax = PG_GETARG_FLOAT8(4);
	q3c_coord_t axis_ratio = PG_GETARG_FLOAT8(5);
	q3c_coord_t PA = PG_GETARG_FLOAT8(6);

	PG_RETURN_BOOL(q3c_ellipse_query_match(ra, dec, ra_cen, dec_cen,
										   semimajax, axis_ratio, PA));
}


PG_FUNCTION_INFO_V1(pgq3c_ellipse_query_support);
Datum pgq3c_ellipse_query_support(PG_FUNCTION_ARGS)
{
	Node *rawreq = (Node *) PG_GETARG_POINTER(0);
	Node *ret = NULL;

	if (IsA(rawreq, SupportRequestSimplify))
	{
		SupportRequestSimplify *req = (SupportRequestSimplify *) rawreq;

		ret = (Node *) q3c_build_ellipse_query_simplified_clause(
			req->root, req->fcall->funcid, req->fcall->args);
	}

	PG_RETURN_POINTER(ret);
}


PG_FUNCTION_INFO_V1(pgq3c_poly_query_array);
Datum pgq3c_poly_query_array(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra = PG_GETARG_FLOAT8(0);
	q3c_coord_t dec = PG_GETARG_FLOAT8(1);
	ArrayType *poly_arr = PG_GETARG_ARRAYTYPE_P(2);

	PG_RETURN_BOOL(q3c_poly_query_array_match(fcinfo, ra, dec, poly_arr));
}


PG_FUNCTION_INFO_V1(pgq3c_poly_query_array_real);
Datum pgq3c_poly_query_array_real(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra = PG_GETARG_FLOAT4(0);
	q3c_coord_t dec = PG_GETARG_FLOAT4(1);
	ArrayType *poly_arr = PG_GETARG_ARRAYTYPE_P(2);

	PG_RETURN_BOOL(q3c_poly_query_array_match(fcinfo, ra, dec, poly_arr));
}


PG_FUNCTION_INFO_V1(pgq3c_poly_query_polygon);
Datum pgq3c_poly_query_polygon(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra = PG_GETARG_FLOAT8(0);
	q3c_coord_t dec = PG_GETARG_FLOAT8(1);
	POLYGON *poly = PG_GETARG_POLYGON_P(2);

	PG_RETURN_BOOL(q3c_poly_query_polygon_match(fcinfo, ra, dec, poly));
}


PG_FUNCTION_INFO_V1(pgq3c_poly_query_polygon_real);
Datum pgq3c_poly_query_polygon_real(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra = PG_GETARG_FLOAT4(0);
	q3c_coord_t dec = PG_GETARG_FLOAT4(1);
	POLYGON *poly = PG_GETARG_POLYGON_P(2);

	PG_RETURN_BOOL(q3c_poly_query_polygon_match(fcinfo, ra, dec, poly));
}


PG_FUNCTION_INFO_V1(pgq3c_poly_query_support);
Datum pgq3c_poly_query_support(PG_FUNCTION_ARGS)
{
	Node *rawreq = (Node *) PG_GETARG_POINTER(0);
	Node *ret = NULL;

	if (IsA(rawreq, SupportRequestSimplify))
	{
		SupportRequestSimplify *req = (SupportRequestSimplify *) rawreq;

		ret = (Node *) q3c_build_poly_query_simplified_clause(
			req->root, req->fcall->funcid, req->fcall->args);
	}

	PG_RETURN_POINTER(ret);
}


PG_FUNCTION_INFO_V1(pgq3c_ellipse_query_it);
Datum pgq3c_ellipse_query_it(PG_FUNCTION_ARGS)
{
	q3c_legacy_iterator_error("q3c_ellipse_query_it");
	PG_RETURN_NULL();
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
	Oid element_type = FLOAT8OID;
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

	if ((poly_nitems % 2) != 0)
	{
		elog(ERROR, "Invalid array argument!\nThe array should contain even number of elements");
	}
	else if (poly_nitems <= 4)
	{
		elog(ERROR, "Invalid polygon! The polygon must have more than two vertices");
	}
	else if (poly_nitems > (2 * Q3C_MAX_N_POLY_VERTEX))
	{
		elog(ERROR,"Polygons with more than 100 vertices are not supported");
	}
	p = ARR_DATA_PTR(poly_arr);
	poly_nitems /= 2;
	*nvert = poly_nitems;
	identical = 1;

	bitmap = ARR_NULLBITMAP(poly_arr);
	bitmask = 1;

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
	else if (npts > Q3C_MAX_N_POLY_VERTEX)
	{
		elog(ERROR,"Polygons with more than 100 vertices are not supported");
	}

	for(i = 0; i < npts; i++)
	{
		newx = poly->p[i].x;
		newy = poly->p[i].y;
		if ((newx != ra[i]) || (newy != dec[i])) {identical = 0;}
		ra[i] = newx;
		dec[i] = newy;
	}
	return identical;
}


typedef struct q3c_poly_info_type {
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

static bool
q3c_poly_query_array_match(FunctionCallInfo fcinfo, q3c_coord_t ra_cen,
						   q3c_coord_t dec_cen, ArrayType *poly_arr)
{
	extern struct q3c_prm hprm;
	char too_large = 0;
	int nvert;
	int identical;
	q3c_poly_info_type *qpit;

	if (fcinfo->flinfo->fn_extra == 0)
	{
		fcinfo->flinfo->fn_extra =
			MemoryContextAlloc(fcinfo->flinfo->fn_mcxt,
							 sizeof(q3c_poly_info_type));
		((q3c_poly_info_type *) fcinfo->flinfo->fn_extra)->ready = 0;
	}

	qpit = (q3c_poly_info_type *) fcinfo->flinfo->fn_extra;
	identical = convert_pgarray2poly(poly_arr, qpit->ra, qpit->dec, &nvert) &&
		qpit->ready;

	if (q3c_check_sphere_point_in_poly(&hprm, nvert, qpit->ra, qpit->dec,
									   ra_cen, dec_cen, &too_large, identical,
									   qpit->xpj, qpit->ypj,
									   qpit->axpj, qpit->aypj,
									   qpit->faces,
									   &(qpit->multi_flag)) == Q3C_DISJUNCT)
	{
		if (too_large)
		{
			elog(ERROR, "The polygon is too large. Polygons having diameter >~23 degrees are unsupported");
		}

		qpit->ready = 1;
		return false;
	}

	if (too_large)
	{
		elog(ERROR, "The polygon is too large. Polygons having diameter >~23 degrees are unsupported");
	}
	qpit->ready = 1;
	return true;
}


static bool
q3c_poly_query_polygon_match(FunctionCallInfo fcinfo, q3c_coord_t ra_cen,
							 q3c_coord_t dec_cen, POLYGON *poly)
{
	extern struct q3c_prm hprm;
	char too_large = 0;
	int nvert;
	int identical;
	q3c_poly_info_type *qpit;

	if (fcinfo->flinfo->fn_extra == 0)
	{
		fcinfo->flinfo->fn_extra =
			MemoryContextAlloc(fcinfo->flinfo->fn_mcxt,
							 sizeof(q3c_poly_info_type));
		((q3c_poly_info_type *) fcinfo->flinfo->fn_extra)->ready = 0;
	}

	qpit = (q3c_poly_info_type *) fcinfo->flinfo->fn_extra;
	identical = convert_pgpoly2poly(poly, qpit->ra, qpit->dec, &nvert) &&
		qpit->ready;

	if (q3c_check_sphere_point_in_poly(&hprm, nvert, qpit->ra, qpit->dec,
									   ra_cen, dec_cen, &too_large, identical,
									   qpit->xpj, qpit->ypj,
									   qpit->axpj, qpit->aypj,
									   qpit->faces,
									   &(qpit->multi_flag)) == Q3C_DISJUNCT)
	{
		if (too_large)
		{
			elog(ERROR, "The polygon is too large. Polygons having diameter >~23 degrees are unsupported");
		}

		qpit->ready = 1;
		return false;
	}

	qpit->ready = 1;
	if (too_large)
	{
		elog(ERROR, "The polygon is too large. Polygons having diameter >~23 degrees are unsupported");
	}

	return true;
}


PG_FUNCTION_INFO_V1(pgq3c_poly_query_it);
Datum pgq3c_poly_query_it(PG_FUNCTION_ARGS)
{
	q3c_legacy_iterator_error("q3c_poly_query_it");
	PG_RETURN_NULL();
}


PG_FUNCTION_INFO_V1(pgq3c_poly_query1_it);
Datum pgq3c_poly_query1_it(PG_FUNCTION_ARGS)
{
	q3c_legacy_iterator_error("q3c_poly_query_it");
	PG_RETURN_NULL();
}


PG_FUNCTION_INFO_V1(pgq3c_in_ellipse);
Datum pgq3c_in_ellipse(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra = PG_GETARG_FLOAT8(0); // ra_cen
	q3c_coord_t dec = PG_GETARG_FLOAT8(1); // dec_cen
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(2); // ra_cen
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(3); // dec_cen
	q3c_coord_t semimajax = PG_GETARG_FLOAT8(4); // semi-major axis
	q3c_coord_t axis_ratio = PG_GETARG_FLOAT8(5); // axis_ratio
	q3c_coord_t PA = PG_GETARG_FLOAT8(6); // PA

	PG_RETURN_BOOL(q3c_ellipse_query_match(ra, dec, ra_cen, dec_cen,
										   semimajax, axis_ratio, PA));
}


/* The caching assumptions for this code are
   For the following calls of the function, if the polygon is
   exactly the same in the I will not reproject
 */
PG_FUNCTION_INFO_V1(pgq3c_in_poly);
Datum pgq3c_in_poly(PG_FUNCTION_ARGS)
{
	q3c_legacy_iterator_error("q3c_in_poly");
	PG_RETURN_NULL();
}


PG_FUNCTION_INFO_V1(pgq3c_poly_query_exact_array);
Datum pgq3c_poly_query_exact_array(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(0); // ra_cen
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(1); // dec_cen
	ArrayType *poly_arr = PG_GETARG_ARRAYTYPE_P(2); // ra_cen

	PG_RETURN_BOOL(q3c_poly_query_array_match(fcinfo, ra_cen, dec_cen,
											  poly_arr));
}


PG_FUNCTION_INFO_V1(pgq3c_poly_query_exact_array_real);
Datum pgq3c_poly_query_exact_array_real(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra_cen = PG_GETARG_FLOAT4(0); // ra_cen
	q3c_coord_t dec_cen = PG_GETARG_FLOAT4(1); // dec_cen
	ArrayType *poly_arr = PG_GETARG_ARRAYTYPE_P(2); // ra_cen

	PG_RETURN_BOOL(q3c_poly_query_array_match(fcinfo, ra_cen, dec_cen,
											  poly_arr));
}


PG_FUNCTION_INFO_V1(pgq3c_poly_query_exact_polygon);
Datum pgq3c_poly_query_exact_polygon(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra_cen = PG_GETARG_FLOAT8(0); // ra_cen
	q3c_coord_t dec_cen = PG_GETARG_FLOAT8(1); // dec_cen
	POLYGON *poly = PG_GETARG_POLYGON_P(2); // ra_cen

	PG_RETURN_BOOL(q3c_poly_query_polygon_match(fcinfo, ra_cen, dec_cen,
													poly));
}


PG_FUNCTION_INFO_V1(pgq3c_in_poly1);
Datum pgq3c_in_poly1(PG_FUNCTION_ARGS)
{
	q3c_legacy_iterator_error("q3c_in_poly");
	PG_RETURN_NULL();
}


PG_FUNCTION_INFO_V1(pgq3c_poly_query_exact_polygon_real);
Datum pgq3c_poly_query_exact_polygon_real(PG_FUNCTION_ARGS)
{
	q3c_coord_t ra_cen = PG_GETARG_FLOAT4(0); // ra_cen
	q3c_coord_t dec_cen = PG_GETARG_FLOAT4(1); // dec_cen
	POLYGON *poly = PG_GETARG_POLYGON_P(2); // ra_cen

	PG_RETURN_BOOL(q3c_poly_query_polygon_match(fcinfo, ra_cen, dec_cen,
												poly));
}
