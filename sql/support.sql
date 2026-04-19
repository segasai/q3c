EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test WHERE q3c_radial_query(ra, dec, 11, 12, 0.1);

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test WHERE q3c_radial_query(ra::real, dec::real, 11, 12, 0.1);

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE q3c_ellipse_query(ra, dec, 171.89, -85.71, 0.1, 0.7, 10);

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE q3c_ellipse_join(171.89, -85.71, ra, dec, 0.1, 0.7, 10);

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE q3c_poly_query(ra, dec,
	ARRAY[167.124000,20.711000,177.500000,32.598000,169.530000,34.745000]);

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE q3c_poly_query(ra::real, dec::real,
	ARRAY[167.124000,20.711000,177.500000,32.598000,169.530000,34.745000]);

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE q3c_poly_query(ra, dec,
	'((167.124000,20.711000),(177.500000,32.598000),(169.530000,34.745000))'::polygon);

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE q3c_poly_query(ra::real, dec::real,
	'((167.124000,20.711000),(177.500000,32.598000),(169.530000,34.745000))'::polygon);

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*) FROM test
WHERE q3c_join(171.89, -85.71, ra, dec, 0.1);

SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_seqscan = off;

DROP INDEX q3c_idx_small;

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*)
FROM test_small a
CROSS JOIN LATERAL (
	SELECT 1
	FROM test b
	WHERE q3c_join(a.ra, a.dec, b.ra, b.dec, 0.01)
) AS s;

CREATE INDEX q3c_idx_small ON test_small (q3c_ang2ipix(ra,dec));
ANALYZE test_small;
DROP INDEX q3c_idx;

EXPLAIN (VERBOSE, COSTS OFF)
SELECT count(*)
FROM test a
CROSS JOIN LATERAL (
	SELECT 1
	FROM test_small b
	WHERE q3c_join(a.ra, a.dec, b.ra, b.dec, 0.01)
) AS s;

CREATE INDEX q3c_idx ON test (q3c_ang2ipix(ra,dec));
ANALYZE test;

RESET enable_seqscan;
RESET enable_mergejoin;
RESET enable_hashjoin;
