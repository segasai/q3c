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
        LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION q3c_ang2ipix (double precision, double precision)
	IS 'Function converting Ra and Dec to the Q3C ipix value';

CREATE OR REPLACE FUNCTION q3c_ang2ipix(ra real, decl real)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_ang2ipix_real'
        LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION q3c_ang2ipix(real, real)
	IS 'Function converting Ra and Dec(floats) to the Q3C ipix value';

CREATE OR REPLACE FUNCTION q3c_ipix2ang(ipix bigint)
        RETURNS double precision[]
        AS 'MODULE_PATHNAME', 'pgq3c_ipix2ang'
        LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION q3c_ipix2ang (bigint)
	IS 'Function converting the Q3C ipix value to Ra, Dec';

CREATE OR REPLACE FUNCTION q3c_pixarea(ipix bigint, depth int)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_pixarea'
        LANGUAGE C IMMUTABLE STRICT;
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
        LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION q3c_dist(double precision, double precision,
				double precision, double precision)
	IS 'Function q3c_dist(ra1, dec1, ra2, dec2) computing the distance between points (ra1, dec1) and (ra2, dec2)';

CREATE OR REPLACE FUNCTION q3c_sindist(double precision, double precision,
                                       double precision, double precision)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_sindist'
        LANGUAGE C IMMUTABLE STRICT COST 100;

CREATE OR REPLACE FUNCTION q3c_sindist_pm(
       ra1 double precision, dec1 double precision,
       pmra1 double precision, pmdec1 double precision,
       epoch1 double precision, ra2 double precision, dec2 double precision,
       epoch2 double precision)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_sindist_pm'
        LANGUAGE C IMMUTABLE COST 100;

-- Importantly this is not strict 
CREATE OR REPLACE FUNCTION q3c_dist_pm(
       ra1 double precision, dec1 double precision,
       pmra1 double precision, pmdec1 double precision,
       epoch1 double precision, ra2 double precision, dec2 double precision,
       epoch2 double precision)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_dist_pm'
        LANGUAGE C IMMUTABLE COST 100;
-- Importantly this is not strict 

CREATE OR REPLACE FUNCTION q3c_nearby_it(double precision, double precision, 
					 double precision, integer)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_nearby_it'
        LANGUAGE C IMMUTABLE STRICT COST 100;

CREATE OR REPLACE FUNCTION q3c_nearby_pm_it(
       ra1 double precision, dec1 double precision, 
       pmra1 double precision, pmdec1 double precision,  
       maxepoch_delta double precision, rad double precision, flag integer)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_nearby_pm_it'
        LANGUAGE C IMMUTABLE COST 100; 
-- Importantly this is NOT as strict function because we accept nulls as pms

CREATE OR REPLACE FUNCTION q3c_ellipse_nearby_it(double precision, double precision, 
		double precision, double precision, double precision, integer)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_ellipse_nearby_it'
        LANGUAGE C IMMUTABLE STRICT COST 100;

CREATE OR REPLACE FUNCTION q3c_in_ellipse(ra0 double precision, dec0 double precision,
					ra_ell double precision, dec_ell double precision,
					maj_ax double precision, axis_ratio double precision,
					pa double precision)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_in_ellipse'
        LANGUAGE C IMMUTABLE STRICT COST 100;


CREATE OR REPLACE FUNCTION q3c_radial_query_it(double precision, 
					       double precision, 
					       double precision,
					       integer, integer)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_radial_query_it'
        LANGUAGE C IMMUTABLE STRICT;


CREATE OR REPLACE FUNCTION q3c_ellipse_query_it(ra_ell double precision,
												dec_ell double precision,
												majax double precision,
												axis_ratio double precision,
												PA double precision,
												iteration integer,
												full_flag integer)
	RETURNS bigint
	AS 'MODULE_PATHNAME', 'pgq3c_ellipse_query_it'
LANGUAGE C IMMUTABLE STRICT;


CREATE OR REPLACE FUNCTION q3c_poly_query_it(double precision[], integer,
                                             integer)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_poly_query_it'
        LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION q3c_in_poly(double precision, double precision,
				       double precision[])
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_in_poly'
        LANGUAGE C IMMUTABLE STRICT;


CREATE OR REPLACE FUNCTION q3c_join(leftra double precision, leftdec double precision,
				    rightra double precision, rightdec double precision,
				    radius double precision)
        RETURNS boolean AS
'
SELECT (((q3c_ang2ipix($3,$4)>=(q3c_nearby_it($1,$2,$5,0))) AND (q3c_ang2ipix($3,$4)<=(q3c_nearby_it($1,$2,$5,1))))
    OR ((q3c_ang2ipix($3,$4)>=(q3c_nearby_it($1,$2,$5,2))) AND (q3c_ang2ipix($3,$4)<=(q3c_nearby_it($1,$2,$5,3))))
    OR ((q3c_ang2ipix($3,$4)>=(q3c_nearby_it($1,$2,$5,4))) AND (q3c_ang2ipix($3,$4)<=(q3c_nearby_it($1,$2,$5,5))))
    OR ((q3c_ang2ipix($3,$4)>=(q3c_nearby_it($1,$2,$5,6))) AND (q3c_ang2ipix($3,$4)<=(q3c_nearby_it($1,$2,$5,7))))) 
    AND q3c_sindist($1,$2,$3,$4)<POW(SIN(RADIANS($5)/2),2)
    AND ($5::double precision ==<<>>== ($1,$2,$3,$4)::q3c_type)
' LANGUAGE SQL IMMUTABLE;

 
CREATE OR REPLACE FUNCTION q3c_join(leftra double precision, leftdec double precision,
				    rightra real, rightdec real,
				    radius double precision)
        RETURNS boolean AS
'
SELECT (((q3c_ang2ipix($3,$4)>=(q3c_nearby_it($1,$2,$5,0))) AND (q3c_ang2ipix($3,$4)<=(q3c_nearby_it($1,$2,$5,1))))
    OR ((q3c_ang2ipix($3,$4)>=(q3c_nearby_it($1,$2,$5,2))) AND (q3c_ang2ipix($3,$4)<=(q3c_nearby_it($1,$2,$5,3))))
    OR ((q3c_ang2ipix($3,$4)>=(q3c_nearby_it($1,$2,$5,4))) AND (q3c_ang2ipix($3,$4)<=(q3c_nearby_it($1,$2,$5,5))))
    OR ((q3c_ang2ipix($3,$4)>=(q3c_nearby_it($1,$2,$5,6))) AND (q3c_ang2ipix($3,$4)<=(q3c_nearby_it($1,$2,$5,7))))) 
    AND q3c_sindist($1,$2,$3,$4)<POW(SIN(RADIANS($5)/2),2)
    AND ($5::double precision ==<<>>== ($1,$2,$3,$4)::q3c_type)

' LANGUAGE SQL IMMUTABLE;



CREATE OR REPLACE FUNCTION q3c_join_pm(
       left_ra double precision,  -- 1
       left_dec double precision,  -- 2
       left_pmra double precision, -- 3
       left_pmdec double precision, -- 4
       left_epoch  double precision, -- 5
       right_ra double precision, -- 6
       right_dec double precision, -- 7
       right_epoch double precision, -- 8
       max_epoch_delta double precision, --9 
       radius double precision -- 10
        )

        RETURNS boolean AS
'
SELECT (
       ((q3c_ang2ipix($6,$7) >= q3c_nearby_pm_it($1,$2,$3,$4,$9,$10,0)) AND
       (q3c_ang2ipix($6,$7)  <= q3c_nearby_pm_it($1,$2,$3,$4,$9,$10,1)))
    OR 
       ((q3c_ang2ipix($6,$7) >= q3c_nearby_pm_it($1,$2,$3,$4,$9,$10,2)) AND
       (q3c_ang2ipix($6,$7)  <= q3c_nearby_pm_it($1,$2,$3,$4,$9,$10,3)))
    OR 
       ((q3c_ang2ipix($6,$7) >= q3c_nearby_pm_it($1,$2,$3,$4,$9,$10,4)) AND
       (q3c_ang2ipix($6,$7)  <= q3c_nearby_pm_it($1,$2,$3,$4,$9,$10,5)))
    OR 
       ((q3c_ang2ipix($6,$7) >= q3c_nearby_pm_it($1,$2,$3,$4,$9,$10,6)) AND
       (q3c_ang2ipix($6,$7)  <= q3c_nearby_pm_it($1,$2,$3,$4,$9,$10,7)))) 
    AND q3c_sindist_pm($1,$2,$3,$4,$5,$6,$7,$8)<POW(SIN(RADIANS($10)/2),2)
    AND ($10::double precision ==<<>>== ($1,$2,$6,$7)::q3c_type) 
' LANGUAGE SQL IMMUTABLE;
-- not strict


CREATE OR REPLACE FUNCTION q3c_ellipse_join(leftra double precision, leftdec double precision,
				rightra double precision, rightdec double precision,
				majoraxis double precision, axisratio double precision,
				pa double precision)
        RETURNS boolean AS
'
SELECT (((q3c_ang2ipix($3,$4)>=(q3c_ellipse_nearby_it($1,$2,$5,$6,$7,0))) AND (q3c_ang2ipix($3,$4)<=(q3c_ellipse_nearby_it($1,$2,$5,$6,$7,1))))
    OR ((q3c_ang2ipix($3,$4)>=(q3c_ellipse_nearby_it($1,$2,$5,$6,$7,2))) AND (q3c_ang2ipix($3,$4)<=(q3c_ellipse_nearby_it($1,$2,$5,$6,$7,3))))
    OR ((q3c_ang2ipix($3,$4)>=(q3c_ellipse_nearby_it($1,$2,$5,$6,$7,4))) AND (q3c_ang2ipix($3,$4)<=(q3c_ellipse_nearby_it($1,$2,$5,$6,$7,5))))
    OR ((q3c_ang2ipix($3,$4)>=(q3c_ellipse_nearby_it($1,$2,$5,$6,$7,6))) AND (q3c_ang2ipix($3,$4)<=(q3c_ellipse_nearby_it($1,$2,$5,$6,$7,7))))) 
    AND q3c_in_ellipse($3,$4,$1,$2,$5,$6,$7)
    AND ($5::double precision ==<<>>== ($1,$2,$3,$4)::q3c_type) 
' LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION q3c_radial_query(
                  real, real,
                  double precision, double precision, double precision)
                                         returns boolean as 'SELECT (
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,0,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,1,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,2,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,3,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,4,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,5,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,6,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,7,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,8,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,9,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,10,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,11,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,12,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,13,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,14,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,15,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,16,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,17,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,18,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,19,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,20,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,21,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,22,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,23,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,24,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,25,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,26,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,27,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,28,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,29,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,30,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,31,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,32,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,33,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,34,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,35,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,36,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,37,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,38,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,39,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,40,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,41,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,42,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,43,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,44,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,45,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,46,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,47,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,48,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,49,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,50,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,51,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,52,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,53,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,54,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,55,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,56,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,57,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,58,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,59,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,60,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,61,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,62,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,63,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,64,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,65,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,66,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,67,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,68,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,69,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,70,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,71,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,72,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,73,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,74,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,75,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,76,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,77,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,78,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,79,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,80,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,81,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,82,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,83,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,84,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,85,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,86,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,87,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,88,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,89,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,90,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,91,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,92,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,93,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,94,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,95,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,96,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,97,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,98,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,99,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,0,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,1,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,2,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,3,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,4,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,5,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,6,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,7,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,8,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,9,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,10,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,11,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,12,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,13,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,14,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,15,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,16,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,17,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,18,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,19,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,20,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,21,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,22,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,23,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,24,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,25,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,26,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,27,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,28,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,29,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,30,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,31,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,32,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,33,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,34,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,35,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,36,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,37,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,38,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,39,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,40,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,41,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,42,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,43,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,44,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,45,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,46,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,47,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,48,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,49,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,50,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,51,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,52,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,53,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,54,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,55,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,56,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,57,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,58,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,59,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,60,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,61,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,62,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,63,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,64,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,65,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,66,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,67,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,68,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,69,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,70,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,71,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,72,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,73,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,74,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,75,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,76,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,77,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,78,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,79,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,80,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,81,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,82,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,83,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,84,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,85,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,86,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,87,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,88,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,89,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,90,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,91,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,92,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,93,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,94,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,95,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,96,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,97,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,98,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,99,0)) 
) AND
q3c_sindist($1,$2,$3,$4)<POW(SIN(RADIANS($5)/2),2)
' LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION q3c_radial_query(
                  double precision, double precision,
                  double precision, double precision, double precision)
                                         returns boolean as 'SELECT 
(
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,0,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,1,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,2,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,3,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,4,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,5,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,6,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,7,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,8,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,9,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,10,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,11,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,12,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,13,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,14,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,15,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,16,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,17,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,18,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,19,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,20,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,21,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,22,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,23,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,24,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,25,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,26,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,27,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,28,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,29,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,30,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,31,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,32,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,33,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,34,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,35,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,36,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,37,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,38,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,39,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,40,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,41,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,42,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,43,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,44,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,45,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,46,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,47,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,48,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,49,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,50,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,51,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,52,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,53,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,54,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,55,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,56,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,57,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,58,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,59,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,60,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,61,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,62,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,63,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,64,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,65,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,66,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,67,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,68,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,69,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,70,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,71,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,72,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,73,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,74,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,75,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,76,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,77,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,78,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,79,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,80,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,81,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,82,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,83,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,84,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,85,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,86,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,87,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,88,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,89,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,90,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,91,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,92,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,93,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,94,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,95,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,96,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,97,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,98,1) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,99,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,0,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,1,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,2,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,3,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,4,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,5,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,6,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,7,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,8,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,9,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,10,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,11,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,12,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,13,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,14,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,15,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,16,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,17,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,18,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,19,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,20,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,21,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,22,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,23,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,24,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,25,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,26,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,27,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,28,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,29,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,30,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,31,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,32,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,33,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,34,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,35,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,36,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,37,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,38,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,39,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,40,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,41,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,42,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,43,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,44,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,45,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,46,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,47,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,48,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,49,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,50,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,51,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,52,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,53,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,54,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,55,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,56,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,57,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,58,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,59,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,60,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,61,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,62,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,63,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,64,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,65,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,66,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,67,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,68,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,69,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,70,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,71,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,72,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,73,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,74,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,75,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,76,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,77,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,78,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,79,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,80,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,81,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,82,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,83,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,84,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,85,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,86,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,87,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,88,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,89,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,90,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,91,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,92,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,93,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,94,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,95,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,96,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,97,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_radial_query_it($3,$4,$5,98,0) AND q3c_ang2ipix($1,$2)<q3c_radial_query_it($3,$4,$5,99,0)) 
) AND
q3c_sindist($1,$2,$3,$4)<POW(SIN(RADIANS($5)/2),2)
' LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION q3c_ellipse_query(
                ra_col double precision, dec_col double precision,
                ra_ell double precision, dec_ell double precision,
                majax double precision, axis_ratio double precision,
				PA double precision)
        RETURNS boolean AS 
'SELECT (
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,0,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,1,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,2,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,3,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,4,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,5,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,6,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,7,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,8,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,9,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,10,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,11,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,12,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,13,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,14,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,15,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,16,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,17,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,18,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,19,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,20,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,21,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,22,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,23,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,24,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,25,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,26,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,27,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,28,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,29,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,30,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,31,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,32,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,33,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,34,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,35,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,36,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,37,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,38,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,39,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,40,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,41,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,42,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,43,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,44,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,45,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,46,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,47,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,48,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,49,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,50,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,51,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,52,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,53,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,54,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,55,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,56,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,57,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,58,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,59,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,60,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,61,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,62,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,63,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,64,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,65,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,66,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,67,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,68,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,69,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,70,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,71,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,72,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,73,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,74,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,75,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,76,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,77,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,78,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,79,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,80,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,81,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,82,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,83,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,84,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,85,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,86,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,87,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,88,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,89,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,90,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,91,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,92,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,93,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,94,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,95,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,96,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,97,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,98,1) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,99,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,0,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,1,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,2,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,3,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,4,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,5,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,6,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,7,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,8,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,9,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,10,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,11,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,12,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,13,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,14,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,15,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,16,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,17,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,18,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,19,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,20,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,21,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,22,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,23,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,24,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,25,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,26,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,27,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,28,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,29,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,30,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,31,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,32,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,33,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,34,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,35,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,36,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,37,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,38,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,39,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,40,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,41,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,42,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,43,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,44,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,45,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,46,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,47,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,48,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,49,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,50,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,51,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,52,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,53,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,54,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,55,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,56,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,57,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,58,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,59,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,60,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,61,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,62,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,63,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,64,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,65,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,66,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,67,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,68,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,69,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,70,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,71,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,72,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,73,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,74,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,75,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,76,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,77,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,78,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,79,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,80,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,81,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,82,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,83,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,84,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,85,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,86,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,87,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,88,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,89,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,90,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,91,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,92,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,93,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,94,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,95,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,96,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,97,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_ellipse_query_it($3,$4,$5,$6,$7,98,0) AND q3c_ang2ipix($1,$2)<q3c_ellipse_query_it($3,$4,$5,$6,$7,99,0)) 
) AND 
q3c_in_ellipse($1,$2,$3,$4,$5,$6,$7)
' LANGUAGE SQL IMMUTABLE;





CREATE OR REPLACE FUNCTION q3c_poly_query(
                double precision, double precision,
                double precision[])
                                       returns boolean as 'SELECT 
(
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,0,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,1,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,2,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,3,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,4,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,5,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,6,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,7,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,8,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,9,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,10,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,11,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,12,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,13,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,14,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,15,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,16,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,17,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,18,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,19,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,20,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,21,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,22,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,23,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,24,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,25,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,26,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,27,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,28,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,29,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,30,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,31,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,32,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,33,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,34,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,35,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,36,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,37,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,38,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,39,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,40,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,41,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,42,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,43,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,44,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,45,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,46,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,47,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,48,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,49,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,50,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,51,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,52,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,53,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,54,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,55,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,56,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,57,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,58,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,59,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,60,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,61,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,62,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,63,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,64,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,65,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,66,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,67,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,68,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,69,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,70,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,71,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,72,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,73,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,74,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,75,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,76,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,77,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,78,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,79,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,80,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,81,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,82,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,83,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,84,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,85,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,86,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,87,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,88,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,89,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,90,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,91,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,92,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,93,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,94,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,95,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,96,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,97,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,98,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,99,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,0,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,1,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,2,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,3,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,4,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,5,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,6,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,7,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,8,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,9,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,10,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,11,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,12,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,13,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,14,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,15,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,16,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,17,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,18,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,19,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,20,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,21,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,22,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,23,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,24,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,25,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,26,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,27,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,28,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,29,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,30,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,31,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,32,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,33,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,34,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,35,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,36,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,37,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,38,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,39,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,40,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,41,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,42,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,43,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,44,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,45,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,46,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,47,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,48,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,49,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,50,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,51,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,52,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,53,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,54,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,55,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,56,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,57,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,58,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,59,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,60,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,61,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,62,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,63,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,64,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,65,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,66,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,67,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,68,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,69,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,70,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,71,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,72,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,73,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,74,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,75,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,76,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,77,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,78,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,79,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,80,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,81,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,82,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,83,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,84,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,85,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,86,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,87,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,88,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,89,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,90,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,91,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,92,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,93,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,94,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,95,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,96,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,97,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,98,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,99,0)) 
) AND 
q3c_in_poly($1,$2,$3);
' LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION q3c_poly_query(
                real, real,
                double precision[])
                                       returns boolean as 'SELECT 
(
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,0,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,1,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,2,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,3,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,4,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,5,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,6,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,7,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,8,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,9,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,10,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,11,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,12,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,13,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,14,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,15,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,16,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,17,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,18,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,19,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,20,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,21,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,22,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,23,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,24,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,25,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,26,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,27,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,28,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,29,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,30,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,31,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,32,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,33,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,34,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,35,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,36,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,37,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,38,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,39,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,40,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,41,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,42,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,43,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,44,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,45,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,46,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,47,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,48,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,49,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,50,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,51,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,52,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,53,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,54,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,55,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,56,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,57,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,58,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,59,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,60,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,61,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,62,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,63,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,64,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,65,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,66,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,67,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,68,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,69,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,70,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,71,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,72,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,73,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,74,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,75,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,76,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,77,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,78,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,79,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,80,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,81,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,82,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,83,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,84,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,85,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,86,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,87,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,88,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,89,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,90,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,91,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,92,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,93,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,94,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,95,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,96,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,97,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,98,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,99,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,0,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,1,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,2,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,3,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,4,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,5,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,6,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,7,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,8,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,9,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,10,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,11,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,12,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,13,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,14,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,15,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,16,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,17,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,18,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,19,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,20,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,21,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,22,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,23,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,24,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,25,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,26,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,27,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,28,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,29,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,30,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,31,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,32,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,33,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,34,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,35,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,36,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,37,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,38,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,39,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,40,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,41,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,42,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,43,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,44,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,45,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,46,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,47,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,48,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,49,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,50,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,51,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,52,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,53,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,54,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,55,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,56,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,57,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,58,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,59,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,60,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,61,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,62,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,63,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,64,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,65,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,66,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,67,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,68,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,69,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,70,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,71,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,72,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,73,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,74,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,75,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,76,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,77,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,78,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,79,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,80,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,81,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,82,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,83,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,84,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,85,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,86,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,87,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,88,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,89,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,90,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,91,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,92,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,93,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,94,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,95,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,96,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,97,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,98,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,99,0)) 
) AND 
q3c_in_poly($1,$2,$3) ;
' LANGUAGE SQL IMMUTABLE;

