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
