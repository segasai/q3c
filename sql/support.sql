EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test WHERE q3c_radial_query(ra, dec, 11, 12, 0.1);
