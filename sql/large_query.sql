-- Joins with large search regions (>30 degrees) that spread over several cube
-- faces. These used to lose rows: (1) q3c_get_nearby() did not clamp the box
-- of the main face, so a box wider than the whole face produced an invalid
-- (negative) shift and an empty ipix range for that face; (2) an ellipse
-- projecting as a degenerate curve (hyperbola) on one of the faces got an
-- inverted bounding box, covering a single pixel instead of the face.
-- For each region we print the number of rows inside it (using the same exact
-- predicate the indexed queries filter with), followed by the number of those
-- rows that the join and/or the query misses. The count after the first one
-- must always be zero.
select count(*) from test where q3c_sindist(265.570896,-44.730914,ra,dec)<pow(sin(radians(33.956956)/2),2);
select count(*) from test where q3c_sindist(265.570896,-44.730914,ra,dec)<pow(sin(radians(33.956956)/2),2) and not q3c_join(265.570896,-44.730914,ra,dec,33.956956);
select count(*) from test where q3c_sindist(265.570896,-44.730914,ra,dec)<pow(sin(radians(33.956956)/2),2) and not q3c_radial_query(ra,dec,265.570896,-44.730914,33.956956);
select count(*) from test where q3c_sindist(313.359065,0.104946,ra,dec)<pow(sin(radians(34.611497)/2),2);
select count(*) from test where q3c_sindist(313.359065,0.104946,ra,dec)<pow(sin(radians(34.611497)/2),2) and not q3c_join(313.359065,0.104946,ra,dec,34.611497);
select count(*) from test where q3c_sindist(313.359065,0.104946,ra,dec)<pow(sin(radians(34.611497)/2),2) and not q3c_radial_query(ra,dec,313.359065,0.104946,34.611497);
select count(*) from test where q3c_sindist(139.214616,-0.173254,ra,dec)<pow(sin(radians(33.403101)/2),2);
select count(*) from test where q3c_sindist(139.214616,-0.173254,ra,dec)<pow(sin(radians(33.403101)/2),2) and not q3c_join(139.214616,-0.173254,ra,dec,33.403101);
select count(*) from test where q3c_sindist(139.214616,-0.173254,ra,dec)<pow(sin(radians(33.403101)/2),2) and not q3c_radial_query(ra,dec,139.214616,-0.173254,33.403101);
-- Cones whose projection on the main cube face spills beyond three of its
-- sides. Only two of those overflows correspond to really covered faces (a
-- cone smaller than 35 degrees cannot touch more than three faces), but
-- q3c_multi_face_check() used to pick the two neighbours by the fixed side
-- order, sometimes probing a face the cone does not touch and dropping one
-- it does. Now every overflowing side contributes its neighbour face.
select count(*) from test where q3c_sindist(175.149000,-46.857800,ra,dec)<pow(sin(radians(34.114400)/2),2);
select count(*) from test where q3c_sindist(175.149000,-46.857800,ra,dec)<pow(sin(radians(34.114400)/2),2) and not q3c_join(175.149000,-46.857800,ra,dec,34.114400);
select count(*) from test where q3c_sindist(175.149000,-46.857800,ra,dec)<pow(sin(radians(34.114400)/2),2) and not q3c_radial_query(ra,dec,175.149000,-46.857800,34.114400);
select count(*) from test where q3c_sindist(45.000000,3.000000,ra,dec)<pow(sin(radians(33.000000)/2),2);
select count(*) from test where q3c_sindist(45.000000,3.000000,ra,dec)<pow(sin(radians(33.000000)/2),2) and not q3c_join(45.000000,3.000000,ra,dec,33.000000);
select count(*) from test where q3c_sindist(45.000000,3.000000,ra,dec)<pow(sin(radians(33.000000)/2),2) and not q3c_radial_query(ra,dec,45.000000,3.000000,33.000000);
select count(*) from test where q3c_in_ellipse(ra,dec,142.587000,-64.532000,31.890200,0.265100,63.361300);
select count(*) from test where q3c_in_ellipse(ra,dec,142.587000,-64.532000,31.890200,0.265100,63.361300) and not q3c_ellipse_join(142.587000,-64.532000,ra,dec,31.890200,0.265100,63.361300);
select count(*) from test where q3c_in_ellipse(ra,dec,62.091000,27.288000,30.669000,0.343000,270.434000);
select count(*) from test where q3c_in_ellipse(ra,dec,62.091000,27.288000,30.669000,0.343000,270.434000) and not q3c_ellipse_join(62.091000,27.288000,ra,dec,30.669000,0.343000,270.434000);
select count(*) from test where q3c_in_ellipse(ra,dec,245.704000,66.952000,32.352000,0.186000,326.020000);
select count(*) from test where q3c_in_ellipse(ra,dec,245.704000,66.952000,32.352000,0.186000,326.020000) and not q3c_ellipse_join(245.704000,66.952000,ra,dec,32.352000,0.186000,326.020000);
select count(*) from test where q3c_in_ellipse(ra,dec,43.733000,63.760000,32.854000,0.285000,65.188000);
select count(*) from test where q3c_in_ellipse(ra,dec,43.733000,63.760000,32.854000,0.285000,65.188000) and not q3c_ellipse_join(43.733000,63.760000,ra,dec,32.854000,0.285000,65.188000);
select count(*) from test where q3c_in_ellipse(ra,dec,267.460000,-24.755000,32.795000,0.069000,33.933000);
select count(*) from test where q3c_in_ellipse(ra,dec,267.460000,-24.755000,32.795000,0.069000,33.933000) and not q3c_ellipse_join(267.460000,-24.755000,ra,dec,32.795000,0.069000,33.933000);
select count(*) from test where q3c_in_ellipse(ra,dec,69.871000,-33.860000,31.642000,0.075000,291.857000);
select count(*) from test where q3c_in_ellipse(ra,dec,69.871000,-33.860000,31.642000,0.075000,291.857000) and not q3c_ellipse_join(69.871000,-33.860000,ra,dec,31.642000,0.075000,291.857000);
select count(*) from test where q3c_in_ellipse(ra,dec,219.898000,-11.506000,30.688000,0.053000,359.022000);
select count(*) from test where q3c_in_ellipse(ra,dec,219.898000,-11.506000,30.688000,0.053000,359.022000) and not q3c_ellipse_join(219.898000,-11.506000,ra,dec,30.688000,0.053000,359.022000);
-- Large ellipses whose projection spills beyond three sides of the main face
-- (the same wrong-neighbour-selection problem as the cones above, on the
-- q3c_ellipse_query code path; these two used to lose about half of their
-- rows). Checked against both q3c_ellipse_query and q3c_ellipse_join.
select count(*) from test where q3c_in_ellipse(ra,dec,1.228730,45.230000,34.589200,0.965993,225.656000);
select count(*) from test where q3c_in_ellipse(ra,dec,1.228730,45.230000,34.589200,0.965993,225.656000) and not q3c_ellipse_query(ra,dec,1.228730,45.230000,34.589200,0.965993,225.656000);
select count(*) from test where q3c_in_ellipse(ra,dec,1.228730,45.230000,34.589200,0.965993,225.656000) and not q3c_ellipse_join(1.228730,45.230000,ra,dec,34.589200,0.965993,225.656000);
select count(*) from test where q3c_in_ellipse(ra,dec,133.476000,-0.329908,33.647200,0.878086,93.408900);
select count(*) from test where q3c_in_ellipse(ra,dec,133.476000,-0.329908,33.647200,0.878086,93.408900) and not q3c_ellipse_query(ra,dec,133.476000,-0.329908,33.647200,0.878086,93.408900);
select count(*) from test where q3c_in_ellipse(ra,dec,133.476000,-0.329908,33.647200,0.878086,93.408900) and not q3c_ellipse_join(133.476000,-0.329908,ra,dec,33.647200,0.878086,93.408900);
