\echo Use "CREATE EXTENSION pair" to load this file. \quit

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

CREATE OR REPLACE FUNCTION q3c_nearby_pm_it(
       ra1 double precision, dec1 double precision, 
       pmra1 double precision, pmdec1 double precision,  
       maxepoch_delta double precision, rad double precision, flag integer)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_nearby_pm_it'
        LANGUAGE C IMMUTABLE COST 100; 
-- Importantly this is NOT as strict function because we accept nulls as pms


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
' LANGUAGE SQL IMMUTABLE;
-- not strict

