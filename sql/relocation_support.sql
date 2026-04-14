SET search_path TO public;

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE tests.q3c_radial_query(ra, dec, 171.890000, -85.710000, 0.1);
