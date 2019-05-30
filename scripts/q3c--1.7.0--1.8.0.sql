\echo Use "CREATE EXTENSION q3c" to load this file. \quit

CREATE OR REPLACE FUNCTION q3c_poly_query_it(polygon, integer,
                                             integer)
        RETURNS bigint
        AS 'MODULE_PATHNAME', 'pgq3c_poly_query1_it'
        LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION q3c_in_poly(double precision, double precision,
				       polygon)
        RETURNS boolean
        AS 'MODULE_PATHNAME', 'pgq3c_in_poly1'
        LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION q3c_poly_query(
                double precision, double precision,
                polygon)
                                       returns boolean as 'SELECT 
(
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,0,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,1,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,2,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,3,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,4,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,5,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,6,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,7,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,8,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,9,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,10,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,11,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,12,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,13,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,14,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,15,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,16,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,17,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,18,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,19,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,20,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,21,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,22,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,23,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,24,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,25,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,26,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,27,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,28,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,29,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,30,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,31,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,32,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,33,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,34,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,35,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,36,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,37,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,38,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,39,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,40,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,41,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,42,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,43,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,44,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,45,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,46,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,47,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,48,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,49,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,50,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,51,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,52,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,53,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,54,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,55,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,56,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,57,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,58,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,59,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,60,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,61,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,62,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,63,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,64,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,65,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,66,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,67,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,68,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,69,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,70,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,71,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,72,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,73,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,74,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,75,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,76,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,77,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,78,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,79,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,80,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,81,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,82,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,83,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,84,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,85,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,86,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,87,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,88,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,89,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,90,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,91,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,92,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,93,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,94,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,95,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,96,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,97,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,98,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,99,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,0,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,1,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,2,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,3,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,4,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,5,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,6,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,7,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,8,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,9,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,10,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,11,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,12,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,13,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,14,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,15,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,16,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,17,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,18,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,19,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,20,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,21,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,22,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,23,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,24,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,25,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,26,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,27,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,28,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,29,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,30,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,31,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,32,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,33,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,34,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,35,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,36,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,37,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,38,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,39,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,40,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,41,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,42,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,43,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,44,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,45,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,46,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,47,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,48,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,49,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,50,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,51,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,52,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,53,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,54,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,55,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,56,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,57,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,58,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,59,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,60,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,61,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,62,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,63,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,64,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,65,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,66,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,67,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,68,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,69,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,70,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,71,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,72,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,73,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,74,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,75,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,76,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,77,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,78,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,79,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,80,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,81,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,82,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,83,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,84,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,85,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,86,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,87,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,88,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,89,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,90,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,91,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,92,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,93,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,94,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,95,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,96,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,97,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,98,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,99,0)) 
) AND 
q3c_in_poly($1,$2,$3) ;
' LANGUAGE SQL IMMUTABLE;


CREATE OR REPLACE FUNCTION q3c_poly_query(
                real, real,
                polygon)
                                       returns boolean as 'SELECT 
(
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,0,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,1,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,2,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,3,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,4,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,5,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,6,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,7,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,8,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,9,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,10,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,11,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,12,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,13,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,14,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,15,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,16,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,17,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,18,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,19,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,20,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,21,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,22,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,23,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,24,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,25,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,26,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,27,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,28,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,29,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,30,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,31,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,32,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,33,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,34,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,35,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,36,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,37,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,38,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,39,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,40,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,41,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,42,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,43,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,44,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,45,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,46,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,47,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,48,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,49,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,50,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,51,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,52,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,53,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,54,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,55,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,56,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,57,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,58,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,59,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,60,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,61,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,62,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,63,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,64,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,65,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,66,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,67,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,68,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,69,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,70,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,71,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,72,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,73,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,74,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,75,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,76,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,77,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,78,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,79,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,80,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,81,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,82,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,83,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,84,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,85,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,86,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,87,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,88,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,89,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,90,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,91,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,92,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,93,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,94,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,95,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,96,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,97,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,98,1) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,99,1)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,0,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,1,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,2,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,3,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,4,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,5,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,6,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,7,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,8,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,9,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,10,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,11,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,12,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,13,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,14,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,15,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,16,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,17,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,18,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,19,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,20,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,21,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,22,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,23,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,24,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,25,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,26,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,27,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,28,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,29,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,30,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,31,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,32,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,33,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,34,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,35,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,36,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,37,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,38,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,39,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,40,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,41,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,42,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,43,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,44,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,45,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,46,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,47,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,48,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,49,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,50,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,51,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,52,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,53,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,54,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,55,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,56,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,57,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,58,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,59,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,60,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,61,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,62,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,63,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,64,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,65,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,66,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,67,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,68,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,69,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,70,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,71,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,72,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,73,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,74,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,75,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,76,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,77,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,78,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,79,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,80,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,81,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,82,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,83,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,84,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,85,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,86,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,87,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,88,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,89,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,90,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,91,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,92,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,93,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,94,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,95,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,96,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,97,0)) OR
(q3c_ang2ipix($1,$2)>=q3c_poly_query_it($3,98,0) AND q3c_ang2ipix($1,$2)<q3c_poly_query_it($3,99,0)) 
) AND 
q3c_in_poly($1,$2,$3) ;
' LANGUAGE SQL IMMUTABLE;

