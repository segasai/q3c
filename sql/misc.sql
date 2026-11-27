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
-- NULL cosdec_flag gives NULL instead of reading an unchecked argument (issue #55)
select round(q3c_dist_pm(1,2,3,4,null,2000,1,0,2005)::numeric,10);
select round(q3c_sindist_pm(1,2,3,4,null,2000,1,0,2005)::numeric,10);
-- regions near a cube corner used to overflow the range arrays and raise "too many ipix ranges"
select (select count(*) from test_small where q3c_radial_query(ra, dec, 134.995731603, -35.227917892, 0.698377141)) as n_index, (select count(*) from test_small where q3c_dist(ra, dec, 134.995731603, -35.227917892) < 0.698377141) as n_exact;
select (select count(*) from test_small where q3c_ellipse_query(ra, dec, 315.016928013, -34.307924314, 2.96222095, 0.557272, 266.737)) as n_index, (select count(*) from test_small where q3c_in_ellipse(ra, dec, 315.016928013, -34.307924314, 2.96222095, 0.557272, 266.737)) as n_exact;
select (select count(*) from test_small where q3c_poly_query(ra, dec, ARRAY[133.5,-38, 136.5,-38, 136.5,-35, 133.5,-35])) as n_index, (select count(*) from test_small where q3c_in_poly(ra, dec, ARRAY[133.5,-38, 136.5,-38, 136.5,-35, 133.5,-35])) as n_exact;
-- thin ellipse at a cube corner: q3c_ellipse_join used to miss points on a secondary face
select q3c_in_ellipse(313.590741884, 35.718985187, 315.613281256, 34.023265614, 2.45639473, 0.0205003, 136.404) as in_ell, q3c_ellipse_join(315.613281256, 34.023265614, 313.590741884, 35.718985187, 2.45639473, 0.0205003, 136.404) as joined;
-- 1 mas ellipse: q3c_in_ellipse used to lose precision for tiny ellipses
select q3c_in_ellipse(350.8000002710, 27.3999998611, 350.8, 27.4, 1/3600000.0, 0.5, 30) as outside, q3c_in_ellipse(350.8000000782, 27.4000001203, 350.8, 27.4, 1/3600000.0, 0.5, 30) as inside;
-- issue #60: the region box on a neighbour face touching the far face edge made q3c_get_nearby loop forever (which cones trigger it depends on the libm: the first two on x86_64 glibc, the last two on macOS arm64)
select q3c_join(219.27811033058825, 22.634103768334768, 219.27811033058825, 22.634103768334768, 13.478208875278737) as j1, q3c_join(148.49373479769272, 19.932568534711635, 148.49373479769272, 19.932568534711635, 19.007358406707613) as j2, q3c_join(45.616377184920189, 26.923165993729214, 45.616377184920189, 26.923165993729214, 7.4928631702840676) as j3, q3c_join(312.93494563220764, 38.384335587445804, 312.93494563220764, 38.384335587445804, 3.5266124672562063) as j4;
-- a box side exactly on a quadtree cell boundary made q3c_get_nearby drop a cell of the main face (x86_64 glibc)
select q3c_join(276.02111089691249, -46.584458573904108, 278.4014844202834, -47.603933693246056, 2.0660123695071788) as joined;
-- a region smaller than a pixel is covered by the single pixel containing it
select q3c_join(10, 20, 10, 20, 1e-9) as tiny;
-- cones and ellipses below ~1e-4 deg used to miss points near their edge, or include outside points as fully covered
select sum((q3c_radial_query(ra, dec, ra0, dec0, r) != (q3c_dist(ra, dec, ra0, dec0) < r))::int) as cone_mismatch, sum((q3c_ellipse_query(ra, dec, ra0, dec0, r, 0.5, pa) != q3c_in_ellipse(ra, dec, ra0, dec0, r, 0.5, pa))::int) as ellipse_mismatch from (select ra0, dec0, r, pa, ra0 + f * r * sin(radians(t)) / cos(radians(dec0)) as ra, dec0 + f * r * cos(radians(t)) as dec from (select (i * 137.508) % 360 as ra0, degrees(asin((i * 0.618034) % 2 - 1)) as dec0, 10 ^ (-5 - i % 3) as r, (i * 77.7) % 180 as pa, 0.8 + 0.4 * ((i * 0.7548777) % 1) as f, (i * 222.49) % 360 as t from generate_series(1, 3000) as i) as x) as y;
-- large polygons are clipped to the cube faces: the octant x, y, z > 0 and the lune 0.137 < ra < 160.137, dec > 0
select (select count(*) from test_small where q3c_poly_query(ra, dec, ARRAY[0,0, 90,0, 45,90])) as n_index, (select count(*) from test_small where ra > 0 and ra < 90 and dec > 0) as n_exact;
select (select count(*) from test_small where q3c_poly_query(ra, dec, ARRAY[0.137,0, 80,0, 160.137,0, 160.137,90])) as n_index, (select count(*) from test_small where ra > 0.137 and ra < 160.137 and dec > 0) as n_exact;
select (select count(*) from test_small where q3c_poly_query(ra, dec, '((10,-60),(120,-30),(200,10),(300,-20))'::polygon)) as n_index, (select count(*) from test_small where q3c_in_poly(ra, dec, '((10,-60),(120,-30),(200,10),(300,-20))'::polygon)) as n_exact;
select q3c_in_poly(10, 10, ARRAY[80,0, 0,80, 280,0, 0,-80]) as inside, q3c_in_poly(10, 10, ARRAY[0,-1, 90,-1, 180,-1, 270,-1]) as outside;
