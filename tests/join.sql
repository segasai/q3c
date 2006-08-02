select count(*) from test1 as a, test as b where q3c_join(a.ra,a.dec,b.ra,b.dec,0.002);
select count(*) from test1 as a, test as b where q3c_join(a.ra,a.dec,b.ra,b.dec,0.004);
select count(*) from test1 as a, test as b where q3c_join(a.ra,a.dec,b.ra,b.dec,0.01);
select count(*) from test1 as a, test as b where q3c_join(a.ra,a.dec,b.ra,b.dec,0.02);
select count(*) from test1 as a, test as b where q3c_join(a.ra,a.dec,b.ra,b.dec,0.04);
