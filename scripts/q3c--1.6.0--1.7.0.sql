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
        LEFTARG = double precision, 
        RIGHTARG = q3c_type,
        PROCEDURE = q3c_seloper,
        RESTRICT = q3c_sel,
        JOIN = q3c_seljoin
);

 
 -- A dummy operator function (always returns true)
CREATE OR REPLACE FUNCTION pgq3c_oper(double precision, q3c_type)
        RETURNS bool
        AS 'MODULE_PATHNAME', 'pgq3c_oper'
        LANGUAGE C STRICT IMMUTABLE COST 1000;
 
-- A selectivity function for the q3c operator
CREATE OR REPLACE FUNCTION pgq3c_sel(internal, oid, internal, int4)
        RETURNS float8
        AS 'MODULE_PATHNAME', 'pgq3c_sel'
        LANGUAGE C IMMUTABLE STRICT ;
  

-- distance operator with correct selectivity
CREATE OPERATOR ==<<>>== (
        LEFTARG = double precision,                                                    RIGHTARG = q3c_type,
        PROCEDURE = pgq3c_oper,
        RESTRICT = pgq3c_sel,
	JOIN = q3c_seljoin
);

DROP FUNCTION q3c_radial_query(bigint,
                   double precision, double precision,
                   double precision, double precision, double precision);

DROP FUNCTION q3c_join(double precision, double precision, 
 				    double precision, double precision,
 				    bigint, double precision);


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

