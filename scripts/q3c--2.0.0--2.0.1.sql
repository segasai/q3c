\echo Use "CREATE EXTENSION q3c" to load this file. \quit

# make functions parallel safe

CREATE OR REPLACE FUNCTION q3c_ang2ipix(double precision, double precision)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_ang2ipix'
        LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION q3c_ang2ipix(ra real, decl real)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_ang2ipix_real'
        LANGUAGE C IMMUTABLE STRICT  PARALLEL SAFE;

CREATE OR REPLACE FUNCTION q3c_ipix2ang(ipix bigint)
        RETURNS double precision[]
        AS 'MODULE_PATHNAME', 'pgq3c_ipix2ang'
        LANGUAGE C IMMUTABLE STRICT  PARALLEL SAFE;

CREATE OR REPLACE FUNCTION q3c_pixarea(ipix bigint, depth int)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_pixarea'
        LANGUAGE C IMMUTABLE STRICT  PARALLEL SAFE;

CREATE OR REPLACE FUNCTION q3c_dist(ra1 double precision, dec1 double precision,
									ra2 double precision, dec2 double precision)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_dist'
        LANGUAGE C IMMUTABLE STRICT  PARALLEL SAFE; 

CREATE OR REPLACE FUNCTION q3c_sindist(double precision, double precision,
                                       double precision, double precision)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_sindist'
        LANGUAGE C IMMUTABLE STRICT  PARALLEL SAFE COST 100;

CREATE OR REPLACE FUNCTION q3c_sindist_pm(
       ra1 double precision, dec1 double precision,
       pmra1 double precision, pmdec1 double precision,
       cosdec_flag integer,
       epoch1 double precision, ra2 double precision, dec2 double precision,
       epoch2 double precision)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_sindist_pm'
        LANGUAGE C IMMUTABLE  PARALLEL SAFE COST 100;

CREATE OR REPLACE FUNCTION q3c_dist_pm(
       ra1 double precision, dec1 double precision,
       pmra1 double precision, pmdec1 double precision,
       cosdec_flag int,
       epoch1 double precision, ra2 double precision, dec2 double precision,
       epoch2 double precision)
        RETURNS double precision
        AS 'MODULE_PATHNAME', 'pgq3c_dist_pm'
        LANGUAGE C IMMUTABLE  PARALLEL SAFE COST 100;
