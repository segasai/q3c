[![Build Status](https://github.com/segasai/q3c/workflows/q3c/badge.svg)](https://github.com/segasai/q3c/actions)
[![Coverage Status](https://coveralls.io/repos/github/segasai/q3c/badge.svg?branch=master)](https://coveralls.io/github/segasai/q3c?branch=master)
# Q3C

Author: Sergey Koposov, University of Edinburgh

Copyright (c) 2004-2021 Sergey Koposov

Email:  skoposov AT ed DOT ac DOT uk

Fresh versions of the software could be obtained here: https://github.com/segasai/q3c 

To read more about the Q3C indexing, you can check out the paper published in ADASS conference proceedings 
http://adsabs.harvard.edu/abs/2006ASPC..351..735K
The citation is "Koposov, S., & Bartunov, O. 2006, Astronomical Society of the Pacific Conference Series, 351, 735".
Q3C is also registered in the ASCL library https://ascl.net/1905.008 . If you use Q3C, you are kindly asked to cite the 2006 paper.
I am also always happy to hear about any usage of Q3C.

## Prerequisites

In order to use Q3C you need to have a PostgreSQL database installed (version 9.1
or later). If you have PostgreSQL version lower than 9.1, you will need to use 
an older version of Q3C (1.4.x). 

To successfully compile Q3C you must have pg_config in your PATH (that means that you may need to install the -devel versions of PostgreSQL packages)

## Installation

- make 
- make install
- Execute "CREATE EXTENSION q3c" in the PostgreSQL client(psql) for the database where you plan to use q3c

After the installation you will have several new functions in PostgreSQL.
All names of these functions start with the "q3c_" prefix.

## Updating 

If you are updating from previous version of q3c, you still need to do the make, make install steps, but after that you need to do

- ALTER EXTENSION q3c UPDATE TO 'A.B.C';

instead of 'CREATE EXTENSION'. Here A.B.C is the placeholder for the version, i.e. '2.0.0';
You also may want to check what version of q3c is installed by either of following commands:

- select q3c_version();
- SELECT * FROM pg_available_extension_versions WHERE name ='q3c';

## Table preparation for Q3C

To begin use Q3C for searches and cross-matches you should create the indexes
on your tables.

In this demonstration we'll assume that you have the table called "mytable" with "ra" and "dec" columns (right ascension and declination in degrees).

First, you will need to create the spatial index, using the command:

`my_db# CREATE INDEX ON mytable (q3c_ang2ipix(ra, dec)); `

The next procedure is optional but strongly recommended: cluster the table using newly created index. The clustering procedure is the procedure of ordering the data on the disk according to the Q3C spatial index values, which will ensure faster queries if your table is very large. If the data have been ingested in the database in ordered fashion (i.e. along some spherical zones), the clustering step can be omitted (although still recommended). The clustering step may take a while (hours) if your dataset is large.

`my_db# CLUSTER mytable_q3c_ang2ipix_idx ON mytable;`

Alternatively, instead of CLUSTER, you can also just reorder your table yourself before indexing (can be faster)
`my_db# create table mytable1 as select * from mytable order by q3c_ang2ipix(ra,dec);`

The last step is analyzing your table:

`my_db# ANALYZE mytable;`

Now you should be able to use q3c queries.

## Q3C functions

*IMPORTANT* Throughout q3c it is assumed that all the angles (ra, dec and distances) are in units of angular degrees, the proper motions are in mas/year, and that the units for the epochs are years, i.e. 2000.5, 2010.5.

The functions installed by Q3C are: 

- q3c_ang2ipix(ra, dec) -- returns the ipix value at ra and dec

- q3c_dist(ra1, dec1, ra2, dec2) -- returns the distance in degrees between two  points (ra1,dec1) and (ra2,dec2)

- q3c_dist_pm(ra1, dec1, pmra1, pmdec1, cosdec_flag, epoch1, ra2, dec2, epoch2) -- returns 
  the distance in degrees between two points (ra1,dec1) and (ra2,dec2) at
  the epoch epoch2 while taking the proper motion into account.
  *IMPORTANT* The cosdec flag (0 or 1) indicates whether the provided proper motion
  includes the cos(dec) term (1) or not (0) . The previous versions
  (q3c 1.8) did not have that parameter and assumed pmra without cos(dec))

- q3c_join(ra1, dec1, ra2, dec2, radius)  -- returns true if (ra1, dec1)
  is within radius spherical distance of (ra2, dec2). It should be used when 
  the index on q3c_ang2ipix(ra2, dec2) is created. See below for examples.

- q3c_join_pm(ra1, dec1, pmra1, pmdec1, cosdec_flag, epoch1, 
  		   ra2, dec2, epoch2, max_delta_epoch, radius)  -- returns true   if (ra1, dec1)
  is within radius spherical distance of (ra2, dec2). It takes into account 
  the proper motion of the source pmra1, pmdec1 (in mas/yr) 
  and epochs of the source coordinates epoch1, and epoch2 (in years).
  max_delta_epoch is the maximum epoch difference possible between two
  tables (i.e. if the oldest epoch in catalog1 is 1970 and the newest epoch 
  in catalog2 is 2015, then the max_delta_epoch should be 45). 
  You should use this function if the index on q3c_ang2ipix(ra2,dec2)
  was created. 
  *IMPORTANT* The cosdec flag (0 or 1) indicates whether the provided proper motion
  includes the cos(dec) term (1) or not (0) . The previous versions
  (q3c 1.8) did not have that parameter and assumed pmra without cos(dec))


- q3c_ellipse_join(ra1, dec1, ra2, dec2, major, ratio, pa) -- like
  q3c_join, except (ra1, dec1) have to be within an ellipse with
  semi-major axis major, the axis ratio ratio and the position angle pa
  (from north through east)

- q3c_radial_query(ra, dec, center_ra, center_dec, radius) -- returns
  true if ra, dec is within radius degrees of center_ra, center_dec. 
  This is the main function for cone searches. This function should be used
  when the index on q3c_ang2ipix(ra,dec) is created.

- q3c_ellipse_query(ra, dec, center_ra, center_dec, maj_ax,
						axis_ratio, PA ) -- returns
  true if ra, dec is within the ellipse from center_ra, center_dec.
  The ellipse is specified by semi-major axis, axis ratio and positional angle.
  This function should be used if when the index on q3c_ang2ipix(ra,dec) is created.

- q3c_poly_query(ra, dec, poly) -- returns true if ra, dec is within the 
  spherical polygon specified as an array of right ascensions and declinations
  Alternatively poly can be an PostgreSQL polygon type. This function
  uses the index for faster queries, assuming the index on q3c_ang2ipix(ra,dec)
  was created.

- q3c_ipix2ang(ipix) -- returns a two-element array of (ra,dec) corresponding to a given ipix.

- q3c_pixarea(ipix, bits) -- returns the area corresponding to a given ipix at the pixelisation level given by 
	bits (1 is smallest, 30 is the cube face) in steradians.

- q3c_ipixcenter(ra, dec, bits) -- returns the ipix value of the
	pixel center at certain pixel depth covering the specified (ra,dec)

- q3c_in_poly(ra, dec, poly) -- returns true/false if point is inside a 
  polygon. This function will not use the index.

- q3c_version() -- returns the version of Q3C that is installed


## Query examples

- The cone search (the query of all objects within the circular region of the sky):
  For example to query all objects within radius of 0.1 deg from (ra,dec) = (11,12)deg in the table mytable you would do:
```
my_db# SELECT * FROM mytable WHERE q3c_radial_query(ra, dec, 11, 12, 0.1);
```
The order of arguments is important, so that the column names of the table should come first, and the 
location where you search after, otherwise the index won't be used.

There is also an alternative way of doing cone searches which could be a bit 
faster if the table that you are working with that table that is small. In 
that case q3c_radial_query may be too CPU heavy. So you may want to query the
table:
 
```
  my_db# SELECT * FROM mytable WHERE q3c_join(11, 12, ra, dec, 0.1);
```

- The ellipse search: search for objects within the ellipse from a given point:
```
my_db=# select * from mytable WHERE
	q3c_ellipse_query(ra, dec, 10, 20, 1, 0.5 ,10);
```
returns the objects which are within the ellipse with the center at (ra,dec)=(10,20) semi-major axis of 1 degree, axis ratio of 0.5 and positional angle of 10 degrees.

- The polygonal query, i.e. the query of the objects which lie inside the 
  region  bounded by the polygon on the sphere. 
  To query the objects in the polygon ((0,0),(2,0),(2,1),(0,1)) ) 
  (this is the spherical polygon with following vertices:
  (ra=0, dec=0) ; (ra=2, dec=0); (ra=2, dec=1); (ra=0, dec=1)):

```
my_db# SELECT * FROM mytable WHERE
		q3c_poly_query(ra, dec, ARRAY[0, 0, 2, 0, 2, 1, 0, 1]);
```
- The polygonal query using PostgreSQL polygon type
```
my_db# SELECT * FROM mytable WHERE
		q3c_poly_query(ra, dec, '((0, 0), (2, 0), (2, 1), (0, 1))'::polygon);
```

- The positional cross-match of the tables: 
  In this example we will assume that we have a huge table "table2" with ra 
  and dec columns and an already created index on q3c_ang2ipix(ra,dec) and 
  a smaller table "table1" with ra and dec columns.

  Now, if we want to cross-match the tables "table1" and "table2" by position
  with the crossmatch radius of 0.001 degrees, we would do it with the 
  following query:
  
```
my_db# SELECT * FROM table1 AS a, table2 AS b WHERE
		q3c_join(a.ra, a.dec, b.ra, b.dec, 0.001);
```
  
The order of arguments is important again, because it determines whether an
index is going to be used or not. The ra,dec columns from the table with the 
index should go after the ra,dec columns from the table without the index.

It is important that the query will return *ALL* the pairs within the matching 
distance, rather than just nearest neighbors. See the nearest neighbors queries below.
  
If every object in table1 have his own error circle ( we'll assume 
that the radius of that circle in degrees is stored in the column "err"),
then you should run the query:
  
```
my_db# SELECT * FROM table1 AS a, table2 AS b WHERE
		q3c_join(a.ra, a.dec, b.ra, b.dec, a.err);
```

- The positional cross-match of the tables with the ellipse error-area:
(for example if you want to find all the objects from one catalogue which lies
inside the elliptical bodies of the galaxies from the second catalogue)
  
It is possible to do the join when the error area of each record of the 
catalogue is an ellipse. Then you can do the query like this
```
my_db# SELECT * FROM table1 AS a, table2 AS b WHERE
		q3c_ellipse_join(a.ra, a.dec, b.ra, b.dec, a.maj_ax
		a.axis_ratio, a.PA);
```
where axis_ratio is the column with axis ratio of the ellipses and PA is the 
column with the positional angles of them, and maj_ax is the column with
semi-major axes of those ellipses.

- The positional cross-match of the tables with proper motions taken into
  account
  In this example we will assume that we have a huge table "table2" with ra 
  and dec columns and an already created index on q3c_ang2ipix(ra,dec) and 
  a smaller table "table1" with ra and dec columns. We will also assume that
  this table1 has an epoch column (in year units) as well as pmra, pmdec
  columns (in units of mas/yr), while the table2 only has the epoch column.
  We will also assume that the pmra columns has the cos(dec) factor. 
  and that we know the upper bound on the epoch difference
  between the two catalogs is say 30 years. (it doesn't have to be precise,
  but it is important that the true largest epoch difference is not larger
  than the specified number).

  Now, if we want to cross-match the tables "table1" and "table2" by position
  with the crossmatch radius of 0.001 degrees, we would do it with the 
  following query:
  
```
my_db# SELECT * FROM table1 AS a, table2 AS b WHERE
		q3c_join_pm (a.ra, a.dec, a.pmra, a.pmdec, 1,
		a.epoch, b.ra, b.dec, b.epoch, 30, 0.001);
```
  


- The density estimation of your objects using pixelation depth of 25:
```
my_db# SELECT (q3c_ipix2ang(i))[1] as ra ,(q3c_ipix2ang(i))[2] as dec ,c,
				q3c_pixarea(i,25) as area from 
					(select q3c_ipixcenter(ra,dec, 25) as i, count(*) as c from
						mytable group by i) as x;
```
returns the list of ra,dec of the Q3C pixel center, number of objects
within a given pixel, and pixel area. If you use that query you should
keep in mind that Q3C doesn't have the property of uniform pixel areas (as 
opposed to HEALPIX).

- Nearest neighbor queries: 
This query selects the only nearest neighbor for each row in your table. If there is no neighbor, 
the columns are filled with nulls.
```
my_db# SELECT  t.*, ss.* FROM mytable AS t
       LEFT JOIN LATERAL (
               SELECT s.* 
                    FROM 
                        sdssdr9.phototag AS s
                    WHERE
                        q3c_join(t.ra, t.dec, s.ra, s.dec, 1./3600)
                    ORDER BY
                        q3c_dist(t.ra,t.dec,s.ra,s.dec)
                    ASC LIMIT 1
               ) as ss ON true;
```
The idea behind the query is that for every row of your table LATERAL() executes the subquery, 
that returns all the neihhbours 
within the aperture and then orders them by distance takes the top one.
  
If you want only the objects that have the neighbors then the query will look like that
  
```
my_db# SELECT  t.*, ss.* FROM mytable AS t,
       LATERAL (
               SELECT s.* 
                    FROM 
                        sdssdr9.phototag AS s
                    WHERE
                        q3c_join(t.ra, t.dec, s.ra, s.dec, 1./3600)
                    ORDER BY
                        q3c_dist(t.ra,t.dec,s.ra,s.dec)
                    ASC LIMIT 1
               ) as ss ;
```

-  Nearest neighbor 2 

This query selects the only nearest neighbor for each row in your table. If there are no 
neighbors, the columns are filled with nulls. This query requires presence of some object id column with the index on the table.
```
my_db# WITH x AS MATERIALIZED (
      SELECT *, ( SELECT objid FROM sdssdr9.phototag AS p WHERE q3c_join(m.ra, m.dec, p.ra, p.dec, 1./3600)
                  ORDER BY q3c_dist(m.ra, m.dec, p.ra, p.dec) ASC LIMIT 1) AS match_objid  FROM mytable AS m 
          )
    SELECT * FROM x, sdssdr9.phototag AS s WHERE x.match_objid=s.objid;
```
## Limitations 

- Querying of very large polygons occupying area with the diameter > 25 degrees or so is not supported
- Polygons with more than 100 vertices are not supported

## Perfomance issues/Slow queries

If you experience slow q3c queries, the following list may suggest possible 
solutions. 

- Check that you are using the correct order of arguments in the q3c functions.
  I.e. q3c_radial_query(120,3,ra,dec,1) instead of q3c_radial_query(ra,dec,120,3,1)
- Verify the plan of the query using 'EXPLAIN ...' command. That will tell you
  how PG tries to execute it. If you see something involving merge_join, or 
  just seq scans (instead of bitmap scans using the q3c index), likely the plan is wrong and you have to fix it
- Force postgresql to use the q3c_index by disabling seq scans or merge and hash joins by setting 'set enable_mergejoin to off; set enable_seqscan to off; set enable hashjoin to off;'
- Cluster your table using q3c index to sort your table by position.
- Check if you are using q3c_join() query together with additional clauses. I.e. the query select * from t1, t2 where q3c_join(t1.ra,t1.dec,t2.ra,t2.dec,1./3600) and t1.mag<1 and t2.mag>33  likely will NOT execute properly, you will likely need to rewrite it as 
```
WITH x AS MATERIALIZED (select * from t1 where t1.mag<1) 
   y as (select *, t2.mag as t2mag from x, t2 where q3c_join(t1.ra,t1.dec,t2.ra,t2.dec,1./3600) )
   select * from y where t2mag>33
```  
