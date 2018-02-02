select count(*) from test_pm1 as a, test as b where q3c_join (
       a.ra+a.pmra/3600/1000*(2000-epoch),
       a.dec+a.pmdec/3600/1000*(2000-epoch),
       b.ra,b.dec,0.002);
select count(*) from test_pm1 as a, test as b where q3c_join(
       a.ra+a.pmra/3600/1000*(2000-epoch),
       a.dec+a.pmdec/3600/1000*(2000-epoch),
       b.ra,b.dec,0.004);
select count(*) from test_pm1 as a, test as b where q3c_join(
       a.ra+a.pmra/3600/1000*(2000-epoch),
       a.dec+a.pmdec/3600/1000*(2000-epoch),
       b.ra,b.dec,0.01);
select count(*) from test_pm1 as a, test as b where q3c_join(
       a.ra+a.pmra/3600/1000*(2000-epoch),
       a.dec+a.pmdec/3600/1000*(2000-epoch),
       b.ra,b.dec,0.02);
select count(*) from test_pm1 as a, test as b where q3c_join(
       a.ra+a.pmra/3600/1000*(2000-epoch),
       a.dec+a.pmdec/3600/1000*(2000-epoch),
       b.ra,b.dec,0.042);

select count(*) from test_pm1 as a, test as b where q3c_join_pm (a.ra,a.dec,a.pmra,a.pmdec,a.epoch,b.ra,b.dec,2000,100,0.002);
select count(*) from test_pm1 as a, test as b where q3c_join_pm(a.ra,a.dec,a.pmra,a.pmdec,a.epoch,b.ra,b.dec,2000,100,0.004);
select count(*) from test_pm1 as a, test as b where q3c_join_pm(a.ra,a.dec,a.pmra,a.pmdec,a.epoch,b.ra,b.dec,2000,100,0.01);
select count(*) from test_pm1 as a, test as b where q3c_join_pm(a.ra,a.dec,a.pmra,a.pmdec,a.epoch,b.ra,b.dec,2000,100,0.02);
select count(*) from test_pm1 as a, test as b where q3c_join_pm(a.ra,a.dec,a.pmra,a.pmdec,a.epoch,b.ra,b.dec,2000,100,0.042);
