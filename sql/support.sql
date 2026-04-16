EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test WHERE q3c_radial_query(ra, dec, 11, 12, 0.1);

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE q3c_ellipse_query(ra, dec, 171.89, -85.71, 0.1, 0.7, 10);
