select count(*) from test1 as a, test as b where q3c_join(a.ra,a.dec,b.ra,b.dec,0.002);
select count(*) from test1 as a, test as b where q3c_join(a.ra,a.dec,b.ra,b.dec,0.004);
select count(*) from test1 as a, test as b where q3c_join(a.ra,a.dec,b.ra,b.dec, 0.0100115); -- shift to avoid numerical issues
select count(*) from test1 as a, test as b where q3c_join(a.ra,a.dec,b.ra,b.dec,0.0200015);
select count(*) from test1 as a, test as b where q3c_join(a.ra,a.dec,b.ra,b.dec,0.042);
