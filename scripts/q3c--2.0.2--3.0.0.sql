DROP FUNCTION IF EXISTS q3c_radial_query_it(double precision, double precision,
                                            double precision, integer, integer);
DROP FUNCTION IF EXISTS q3c_ellipse_query_it(double precision, double precision,
                                             double precision, double precision,
                                             double precision, integer, integer);
DROP FUNCTION IF EXISTS q3c_poly_query_it(double precision[], integer,
                                          integer);
DROP FUNCTION IF EXISTS q3c_poly_query_it(polygon, integer, integer);

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

CREATE OR REPLACE FUNCTION q3c_ellipse_join_support(internal)
        RETURNS internal
        AS 'MODULE_PATHNAME', 'pgq3c_ellipse_join_support'
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

DROP FUNCTION q3c_ellipse_join(double precision, double precision,
                               double precision, double precision,
                               double precision, double precision,
                               double precision);

CREATE OR REPLACE FUNCTION q3c_ellipse_join(
                leftra double precision, leftdec double precision,
                rightra double precision, rightdec double precision,
                semimajoraxis double precision, axisratio double precision,
                pa double precision)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_ellipse_join'
        LANGUAGE C IMMUTABLE PARALLEL SAFE
        SUPPORT q3c_ellipse_join_support
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
