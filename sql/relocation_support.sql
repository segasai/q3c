SET search_path TO public;

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE tests.q3c_radial_query(ra, dec, 171.890000, -85.710000, 0.1);

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE tests.q3c_ellipse_query(ra, dec, 171.890000, -85.710000, 0.1, 0.7, 10);

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE tests.q3c_poly_query(ra, dec,
	ARRAY[167.124000,20.711000,177.500000,32.598000,169.530000,34.745000]);

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE tests.q3c_poly_query(ra, dec,
	'((167.124000,20.711000),(177.500000,32.598000),(169.530000,34.745000))'::polygon);
