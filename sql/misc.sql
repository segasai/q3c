select round(q3c_dist(1,2,3,4)::numeric,10);
select round(q3c_dist(null,null,3,4)::numeric,10);
select round(q3c_sindist(1,2,3,4)::numeric,10);
select round(q3c_sindist(null,null,3,4)::numeric,10);
select round(q3c_dist_pm(1,2,3,4,1,2000,1,0,2005)::numeric,10);
select round(q3c_sindist_pm(1,2,3,4,1,2000,1,0,2005)::numeric,10);
select round(q3c_dist_pm(1,2,3,4,1,2000,null,0,2005)::numeric,10);
select round(q3c_sindist_pm(1,2,3,4,1,2000,null,0,2005)::numeric,10);
select round(q3c_dist_pm(1,2,3,null,1,2000,0,0,2005)::numeric,10);
select round(q3c_sindist_pm(1,2,3,null,1,2000,0,0,2005)::numeric,10);
-- tiny cone near the pole used to drop its own centre (issue #51)
select q3c_radial_query(226.88753647119157, 76.232289097380033, 226.88753647119157, 76.232289097380033, 2.677e-08);
-- polygon cache must not reuse a quad's edges for a triangle sharing its first 3 vertices (issue #53)
select q3c_in_poly(9.5, 10.5, p) from (values (ARRAY[9,9, 11,9, 11,11, 9,11]::double precision[]), (ARRAY[9,9, 11,9, 11,11]::double precision[])) as t(p);
select q3c_in_poly(9.5, 10.5, p) from (values ('((9,9),(11,9),(11,11),(9,11))'::polygon), ('((9,9),(11,9),(11,11))'::polygon)) as t(p);
-- per-row polygon: row 1 warms every call site of q3c_poly_query with its ranges, row 2 must not reuse them (issue #53)
select q3c_poly_query(ra, dec, p) from (values (100.0::float8, 10.0::float8, ARRAY[9,9, 11,9, 11,11, 9,11]::double precision[]), (50.0, 50.0, ARRAY[49,49, 51,49, 51,51, 49,51])) as t(ra, dec, p);
select q3c_poly_query(ra, dec, p) from (values (100.0::float8, 10.0::float8, '((9,9),(11,9),(11,11),(9,11))'::polygon), (50.0, 50.0, '((49,49),(51,49),(51,51),(49,51))'::polygon)) as t(ra, dec, p);
-- non-finite point coordinates give NULL, like q3c_ang2ipix (issue #52)
select q3c_in_poly('NaN'::float8, 10, ARRAY[9,9, 11,9, 11,11, 9,11]::double precision[]);
select q3c_in_poly(10, 'Infinity'::float8, '((9,9),(11,9),(11,11),(9,11))'::polygon);
-- non-finite query centre/radius matches nothing instead of erroring, so a NaN row in the left table of q3c_join is skipped (issue #52)
select q3c_radial_query_it(0, 'NaN'::float8, 1, 0, 1);
select q3c_radial_query(10, 10, 10, 10, 'NaN'::float8);
select q3c_ellipse_query_it(0, 0, 1, 'NaN'::float8, 0, 0, 1);
select q3c_ellipse_query(10, 10, 10, 10, 1, 'Infinity'::float8, 0);
select q3c_nearby_it('NaN'::float8, 0, 1, 0), q3c_nearby_it('NaN'::float8, 0, 1, 1);
select l.ra, count(b.*) from (values ('NaN'::float8, 10::float8), (10, 10)) as l(ra, dec) left join test_small b on q3c_join(l.ra, l.dec, b.ra, b.dec, 1) group by l.ra order by l.ra;
