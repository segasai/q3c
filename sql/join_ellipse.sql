select count(*) from test1 as a, test as b where q3c_ellipse_join(a.ra,a.dec,b.ra,b.dec,0.002,1,0);
select count(*) from test1 as a, test as b where q3c_ellipse_join(a.ra,a.dec,b.ra,b.dec,0.004,1,0);
select count(*) from test1 as a, test as b where q3c_ellipse_join(a.ra,a.dec,b.ra,b.dec,0.0100115,1,0);
select count(*) from test1 as a, test as b where q3c_ellipse_join(a.ra,a.dec,b.ra,b.dec,0.0200015,1,0);
select count(*) from test1 as a, test as b where q3c_ellipse_join(a.ra,a.dec,b.ra,b.dec,0.042,1,0);
select q3c_ellipse_join(337411.646566,980062.989043,-747471.184193,82822.425422,-271476.430786,1,0) as q3c_join;
