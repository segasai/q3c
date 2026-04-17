SELECT q3c_radial_query(11.0, 12.0, 11.0 + offs, 12.0, 0.1)
FROM (VALUES (0.0::double precision)) AS v(offs);

SELECT q3c_radial_query(11.0::real, 12.0::real, 11.0 + offs, 12.0, 0.1)
FROM (VALUES (0.0::double precision)) AS v(offs);

SELECT q3c_ellipse_query(170.0, 30.0, 170.0 + offs, 30.0, 0.1, 1.0, 0.0) =
	q3c_in_ellipse(170.0, 30.0, 170.0 + offs, 30.0, 0.1, 1.0, 0.0)
FROM (VALUES (0.0::double precision)) AS v(offs);

SELECT q3c_poly_query(170.0, 30.0, poly) = q3c_in_poly(170.0, 30.0, poly)
FROM (VALUES (ARRAY[167.124000,20.711000,177.500000,32.598000,
					169.530000,34.745000]::double precision[])) AS v(poly);

SELECT q3c_poly_query(170.0::real, 30.0::real, poly) =
	q3c_in_poly(170.0::real, 30.0::real, poly)
FROM (VALUES (ARRAY[167.124000,20.711000,177.500000,32.598000,
					169.530000,34.745000]::double precision[])) AS v(poly);

SELECT q3c_poly_query(170.0, 30.0, poly) = q3c_in_poly(170.0, 30.0, poly)
FROM (VALUES ('((167.124000,20.711000),(177.500000,32.598000),
				(169.530000,34.745000))'::polygon)) AS v(poly);

SELECT q3c_poly_query(170.0::real, 30.0::real, poly) =
	q3c_in_poly(170.0::real, 30.0::real, poly)
FROM (VALUES ('((167.124000,20.711000),(177.500000,32.598000),
				(169.530000,34.745000))'::polygon)) AS v(poly);

SELECT (
	SELECT count(*) FROM test_small
	WHERE q3c_radial_query(ra::real, dec::real, 11, 12, 0.1)
) = (
	SELECT count(*) FROM test_small
	WHERE q3c_radial_query(ra, dec, 11, 12, 0.1)
);

SELECT (
	SELECT count(*) FROM test_small
	WHERE q3c_radial_query_exact(ra::real, dec::real, 11, 12, 0.1)
) = (
	SELECT count(*) FROM test_small
	WHERE q3c_radial_query_exact(ra, dec, 11, 12, 0.1)
);

SELECT (
	SELECT count(*) FROM test_small
	WHERE q3c_poly_query(ra::real, dec::real,
		ARRAY[167.124000,20.711000,177.500000,32.598000,169.530000,34.745000])
) = (
	SELECT count(*) FROM test_small
	WHERE q3c_poly_query(ra, dec,
		ARRAY[167.124000,20.711000,177.500000,32.598000,169.530000,34.745000])
);

SELECT (
	SELECT count(*) FROM test_small
	WHERE q3c_in_poly(ra::real, dec::real,
		ARRAY[167.124000,20.711000,177.500000,32.598000,169.530000,34.745000])
) = (
	SELECT count(*) FROM test_small
	WHERE q3c_in_poly(ra, dec,
		ARRAY[167.124000,20.711000,177.500000,32.598000,169.530000,34.745000])
);

SELECT (
	SELECT count(*) FROM test_small
	WHERE q3c_poly_query(ra::real, dec::real,
		'((167.124000,20.711000),(177.500000,32.598000),(169.530000,34.745000))'::polygon)
) = (
	SELECT count(*) FROM test_small
	WHERE q3c_poly_query(ra, dec,
		'((167.124000,20.711000),(177.500000,32.598000),(169.530000,34.745000))'::polygon)
);

SELECT (
	SELECT count(*) FROM test_small
	WHERE q3c_in_poly(ra::real, dec::real,
		'((167.124000,20.711000),(177.500000,32.598000),(169.530000,34.745000))'::polygon)
) = (
	SELECT count(*) FROM test_small
	WHERE q3c_in_poly(ra, dec,
		'((167.124000,20.711000),(177.500000,32.598000),(169.530000,34.745000))'::polygon)
);
