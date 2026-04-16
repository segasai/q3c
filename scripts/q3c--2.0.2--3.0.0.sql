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
