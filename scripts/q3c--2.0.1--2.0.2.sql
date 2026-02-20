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

