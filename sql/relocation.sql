set search_path to tests,public;
select count(*) from test where q3c_radial_query(ra,dec,171.890000,-85.710000,0.1);
select count(*) from test where q3c_radial_query(ra,dec,45.360000,-8.020000,0.1);
select count(*) from test where q3c_radial_query(ra,dec,247.030000,50.850000,0.1);
select count(*) from test where q3c_radial_query(ra,dec,308.900000,42.260000,0.1);
