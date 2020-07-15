select q3c_ang2ipix(0, 0); 
select q3c_ang2ipix(0, 91); 
select q3c_ang2ipix(0, 90); 
select q3c_ang2ipix(-1, 0); 
select q3c_ang2ipix(359, 0); 
select q3c_ang2ipix('nan'::double precision,0);
select sum((q3c_ang2ipix(((q3c_ipix2ang(q3c_ang2ipix(ra,dec)))[1]),(q3c_ipix2ang(q3c_ang2ipix(ra,dec)))[2])=q3c_ang2ipix(ra,dec))::int) from test;
select sum((q3c_ang2ipix(ra::real,dec::real) = q3c_ang2ipix( (ra::real)::double precision, (dec::real)::double precision))::int) from test;
