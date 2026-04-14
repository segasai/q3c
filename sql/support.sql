EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test WHERE q3c_radial_query(ra, dec, 11, 12, 0.1);

SET plan_cache_mode = force_generic_plan;

PREPARE q3c_support_plan(double precision, double precision, double precision) AS
SELECT count(*) FROM test WHERE q3c_radial_query(ra, dec, $1, $2, $3);

EXPLAIN (VERBOSE, COSTS OFF)
EXECUTE q3c_support_plan(11, 12, 0.1);

DEALLOCATE q3c_support_plan;

RESET plan_cache_mode;
