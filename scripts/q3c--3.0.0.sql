\echo Use "CREATE EXTENSION q3c" to load this file. \quit


-- A dummy type used in the selectivity operator
create type q3c_type as (ra double precision, dec double precision,	
       ra1 double precision, dec1 double precision);


-- A dummy operator function (always returns true)
CREATE OR REPLACE FUNCTION q3c_seloper(double precision, q3c_type)
        RETURNS bool
        AS 'MODULE_PATHNAME', 'pgq3c_seloper'
        LANGUAGE C STRICT IMMUTABLE COST 1000;

-- A selectivity function for the q3c operator
CREATE OR REPLACE FUNCTION q3c_sel(internal, oid, internal, int4)
        RETURNS float8
        AS 'MODULE_PATHNAME', 'pgq3c_sel'
        LANGUAGE C IMMUTABLE STRICT ;
 
-- A selectivity function for the q3c operator
CREATE OR REPLACE FUNCTION q3c_seljoin(internal, oid, internal, int2, internal)
        RETURNS float8
        AS 'MODULE_PATHNAME', 'pgq3c_seljoin'
        LANGUAGE C IMMUTABLE STRICT ;
 

 -- distance operator with correct selectivity
CREATE OPERATOR ==<<>>== (
        LEFTARG = double precision,                                                    RIGHTARG = q3c_type,
        PROCEDURE = q3c_seloper,
        RESTRICT = q3c_sel,
	JOIN = q3c_seljoin
);



CREATE OR REPLACE FUNCTION q3c_version()
        RETURNS cstring
        AS 'MODULE_PATHNAME', 'pgq3c_get_version'
        LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION q3c_version()
	IS 'Function returning Q3C version';


CREATE OR REPLACE FUNCTION q3c_ang2ipix(double precision, double precision)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_ang2ipix'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON FUNCTION q3c_ang2ipix (double precision, double precision)
	IS 'Function converting Ra and Dec to the Q3C ipix value';

CREATE OR REPLACE FUNCTION q3c_ang2ipix(ra real, decl real)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_ang2ipix_real'
        LANGUAGE C IMMUTABLE STRICT  PARALLEL SAFE;
COMMENT ON FUNCTION q3c_ang2ipix(real, real)
	IS 'Function converting Ra and Dec(floats) to the Q3C ipix value';

CREATE OR REPLACE FUNCTION q3c_ipix2ang(ipix bigint)
        RETURNS double precision[]
        AS 'MODULE_PATHNAME', 'pgq3c_ipix2ang'
        LANGUAGE C IMMUTABLE STRICT  PARALLEL SAFE;
COMMENT ON FUNCTION q3c_ipix2ang (bigint)
	IS 'Function converting the Q3C ipix value to Ra, Dec';

CREATE OR REPLACE FUNCTION q3c_pixarea(ipix bigint, depth int)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_pixarea'
        LANGUAGE C IMMUTABLE STRICT  PARALLEL SAFE;
COMMENT ON FUNCTION q3c_pixarea (bigint, int )
	IS 'Function returning the area of the pixel containing ipix being located at certain depth in the quadtree';

CREATE OR REPLACE FUNCTION q3c_ipixcenter(ra double precision, decl double precision, int)
        RETURNS bigint
        AS
   'SELECT ((q3c_ang2ipix($1,$2))>>((2*$3))<<((2*$3))) +
			((1::bigint)<<(2*($3-1))) -1'
	LANGUAGE SQL;

CREATE OR REPLACE FUNCTION q3c_dist(ra1 double precision, dec1 double precision,
									ra2 double precision, dec2 double precision)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_dist'
        LANGUAGE C IMMUTABLE STRICT  PARALLEL SAFE; 
COMMENT ON FUNCTION q3c_dist(double precision, double precision,
				double precision, double precision)
	IS 'Function q3c_dist(ra1, dec1, ra2, dec2) computing the distance between points (ra1, dec1) and (ra2, dec2)';

CREATE OR REPLACE FUNCTION q3c_sindist(double precision, double precision,
                                       double precision, double precision)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_sindist'
        LANGUAGE C IMMUTABLE STRICT  PARALLEL SAFE COST 100;
COMMENT ON FUNCTION q3c_sindist(double precision, double precision,
				double precision, double precision)
	IS 'Function q3c_sindist(ra1, dec1, ra2, dec2) computing the sin(distance/2)^2 between points (ra1, dec1) and (ra2, dec2)';

CREATE OR REPLACE FUNCTION q3c_sindist_pm(
       ra1 double precision, dec1 double precision,
       pmra1 double precision, pmdec1 double precision,
       cosdec_flag integer,
       epoch1 double precision, ra2 double precision, dec2 double precision,
       epoch2 double precision)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_sindist_pm'
        LANGUAGE C IMMUTABLE  PARALLEL SAFE COST 100;

-- Importantly this is not strict 
CREATE OR REPLACE FUNCTION q3c_dist_pm(
       ra1 double precision, dec1 double precision,
       pmra1 double precision, pmdec1 double precision,
       cosdec_flag int,
       epoch1 double precision, ra2 double precision, dec2 double precision,
       epoch2 double precision)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_dist_pm'
        LANGUAGE C IMMUTABLE  PARALLEL SAFE COST 100;
-- Importantly this is not strict 

CREATE OR REPLACE FUNCTION q3c_nearby_it(double precision, double precision, 
					 double precision, integer)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_nearby_it'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE COST 100;

CREATE OR REPLACE FUNCTION q3c_nearby_pm_it(
       ra1 double precision, dec1 double precision, 
       pmra1 double precision, pmdec1 double precision,
       cosdec_flag int,
       maxepoch_delta double precision, rad double precision, flag integer)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_nearby_pm_it'
        LANGUAGE C IMMUTABLE PARALLEL SAFE COST 100; 
-- Importantly this is NOT as strict function because we accept nulls as pms

CREATE OR REPLACE FUNCTION q3c_ellipse_nearby_it(double precision, double precision, 
		double precision, double precision, double precision, integer)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_ellipse_nearby_it'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE COST 100;

CREATE OR REPLACE FUNCTION q3c_in_ellipse(ra0 double precision, dec0 double precision,
					ra_ell double precision, dec_ell double precision,
					semimaj_ax double precision, axis_ratio double precision,
					pa double precision)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_in_ellipse'
        LANGUAGE C IMMUTABLE STRICT COST 100;


CREATE OR REPLACE FUNCTION q3c_radial_query_exact(
                  real, real,
                  double precision, double precision, double precision)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_radial_query_exact_real'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE COST 100;

CREATE OR REPLACE FUNCTION q3c_radial_query_exact(
                  double precision, double precision,
                  double precision, double precision, double precision)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_radial_query_exact'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE COST 100;

CREATE OR REPLACE FUNCTION q3c_radial_query_support(internal)
        RETURNS internal
        AS 'MODULE_PATHNAME', 'pgq3c_radial_query_support'
        LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION q3c_ellipse_query_support(internal)
        RETURNS internal
        AS 'MODULE_PATHNAME', 'pgq3c_ellipse_query_support'
        LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION q3c_ellipse_join_support(internal)
        RETURNS internal
        AS 'MODULE_PATHNAME', 'pgq3c_ellipse_join_support'
        LANGUAGE C IMMUTABLE;
CREATE OR REPLACE FUNCTION q3c_in_poly(double precision, double precision,
				       double precision[])
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_poly_query_exact_array'
        LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION q3c_in_poly(double precision, double precision,
				       polygon)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_poly_query_exact_polygon'
        LANGUAGE C IMMUTABLE STRICT;





CREATE OR REPLACE FUNCTION q3c_join_pm(
       left_ra double precision,  -- 1
       left_dec double precision,  -- 2
       left_pmra double precision, -- 3
       left_pmdec double precision, -- 4
       cosdec_flag int, --5
       left_epoch  double precision, -- 6
       right_ra double precision, -- 7
       right_dec double precision, -- 8
       right_epoch double precision, -- 9
       max_epoch_delta double precision, -- 10
       radius double precision -- 11
        )

        RETURNS boolean AS
'
SELECT (
       ((q3c_ang2ipix($7,$8) >= q3c_nearby_pm_it($1,$2,$3,$4,$5,$10,$11,0)) AND
       (q3c_ang2ipix($7,$8)  <= q3c_nearby_pm_it($1,$2,$3,$4,$5,$10,$11,1)))
    OR 
       ((q3c_ang2ipix($7,$8) >= q3c_nearby_pm_it($1,$2,$3,$4,$5,$10,$11,2)) AND
       (q3c_ang2ipix($7,$8)  <= q3c_nearby_pm_it($1,$2,$3,$4,$5,$10,$11,3)))
    OR 
       ((q3c_ang2ipix($7,$8) >= q3c_nearby_pm_it($1,$2,$3,$4,$5,$10,$11,4)) AND
       (q3c_ang2ipix($7,$8)  <= q3c_nearby_pm_it($1,$2,$3,$4,$5,$10,$11,5)))
    OR 
       ((q3c_ang2ipix($7,$8) >= q3c_nearby_pm_it($1,$2,$3,$4,$5,$10,$11,6)) AND
       (q3c_ang2ipix($7,$8)  <= q3c_nearby_pm_it($1,$2,$3,$4,$5,$10,$11,7)))) 
    AND q3c_sindist_pm($1,$2,$3,$4,$5,$6,$7,$8,$9)<POW(SIN(RADIANS($11)/2),2)
    AND ($10::double precision ==<<>>== ($1,$2,$6,$7)::q3c_type) 
' LANGUAGE SQL IMMUTABLE;
-- not strict


CREATE OR REPLACE FUNCTION q3c_ellipse_join(leftra double precision, leftdec double precision,
				rightra double precision, rightdec double precision,
				semimajoraxis double precision, axisratio double precision,
				pa double precision)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_ellipse_join'
        LANGUAGE C IMMUTABLE PARALLEL SAFE
        SUPPORT q3c_ellipse_join_support
        COST 100;

CREATE OR REPLACE FUNCTION q3c_radial_query(
                  real, real,
                  double precision, double precision, double precision)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_radial_query_real'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE
        SUPPORT q3c_radial_query_support
        COST 100;

CREATE OR REPLACE FUNCTION q3c_radial_query(
                  double precision, double precision,
                  double precision, double precision, double precision)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_radial_query'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE
        SUPPORT q3c_radial_query_support
        COST 100;

CREATE OR REPLACE FUNCTION q3c_ellipse_query(
                ra_col double precision, dec_col double precision,
                ra_ell double precision, dec_ell double precision,
                majax double precision, axis_ratio double precision,
				PA double precision)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_ellipse_query'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE
        SUPPORT q3c_ellipse_query_support
        COST 100;





CREATE OR REPLACE FUNCTION q3c_in_poly(real, real,
				       double precision[])
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_poly_query_exact_array_real'
        LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION q3c_in_poly(real, real,
				       polygon)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_poly_query_exact_polygon_real'
        LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION q3c_poly_query_support(internal)
        RETURNS internal
        AS 'MODULE_PATHNAME', 'pgq3c_poly_query_support'
        LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION q3c_poly_query(
                double precision, double precision,
                double precision[])
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_poly_query_array'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE
        SUPPORT q3c_poly_query_support
        COST 100;

CREATE OR REPLACE FUNCTION q3c_poly_query(
                real, real,
                double precision[])
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_poly_query_array_real'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE
        SUPPORT q3c_poly_query_support
        COST 100;

CREATE OR REPLACE FUNCTION q3c_poly_query(
                double precision, double precision,
                polygon)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_poly_query_polygon'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE
        SUPPORT q3c_poly_query_support
        COST 100;

CREATE OR REPLACE FUNCTION q3c_poly_query(
                real, real,
                polygon)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_poly_query_polygon_real'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE
        SUPPORT q3c_poly_query_support
        COST 100;

CREATE OR REPLACE FUNCTION q3c_join_support(internal)
        RETURNS internal
        AS 'MODULE_PATHNAME', 'pgq3c_join_support'
        LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION q3c_join_internal_support(internal)
        RETURNS internal
        AS 'MODULE_PATHNAME', 'pgq3c_join_internal_support'
        LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION q3c_ellipse_join_internal_support(internal)
        RETURNS internal
        AS 'MODULE_PATHNAME', 'pgq3c_ellipse_join_internal_support'
        LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION q3c_join_internal(
                bigint, bigint,
                double precision, double precision,
                double precision, double precision,
                double precision, integer)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_join_internal'
        LANGUAGE C IMMUTABLE PARALLEL SAFE
        SUPPORT q3c_join_internal_support
        COST 100;

CREATE OR REPLACE FUNCTION q3c_ellipse_join_internal(
                bigint,
                double precision, double precision,
                double precision, double precision,
                double precision, double precision,
                double precision, integer)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_ellipse_join_internal'
        LANGUAGE C IMMUTABLE PARALLEL SAFE
        SUPPORT q3c_ellipse_join_internal_support
        COST 100;

CREATE OR REPLACE FUNCTION q3c_join(
                leftra double precision, leftdec double precision,
                rightra double precision, rightdec double precision,
                radius double precision)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_join'
        LANGUAGE C IMMUTABLE PARALLEL SAFE
        SUPPORT q3c_join_support
        COST 100;

CREATE OR REPLACE FUNCTION q3c_join(
                leftra double precision, leftdec double precision,
                rightra real, rightdec real,
                radius double precision)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_join_real'
        LANGUAGE C IMMUTABLE PARALLEL SAFE
        SUPPORT q3c_join_support
        COST 100;
