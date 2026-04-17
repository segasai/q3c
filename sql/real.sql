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
