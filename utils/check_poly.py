import shapely
import numpy as np,numpy.random
import random
import multiprocessing,sys
from readcol import readcol
from shapely.geometry import Point, Polygon

def torect(ra,dec):
	cd=np.cos(np.deg2rad(dec))
	sd=np.sin(np.deg2rad(dec))
	sr=np.sin(np.deg2rad(ra))
	cr=np.cos(np.deg2rad(ra))
	x,y,z=cd*cr,cd*sr,sd
	return x,y,z

def fromrect(x,y,z):
	ra = np.rad2deg(np.arctan2(y, x))
	dec = np.rad2deg(np.arctan2(z, np.sqrt(x**2+y**2)))
	rad = np.sqrt(x**2+y**2+z**2)
	return ra,dec
	
def within_poly(ra,dec,rapoly,decpoly):
	rapolycen, decpolycen = (rapoly[0]), (decpoly[0])
	x, y, z = torect(ra, dec)
	xpoly,ypoly,zpoly=torect(rapoly,decpoly)
	xpolycen,ypolycen,zpolycen=torect(rapolycen,decpolycen	)
	dists = x*xpolycen+y*ypolycen+z*zpolycen	
	ind = dists>np.cos(np.deg2rad(70)) # only consider the points located at the same
					#hemisphere as the polygon
	x1,y1,z1=x[ind],y[ind],z[ind]
	point = Point(0,0)
	arr = []
	for curx,cury,curz in zip(x1,y1,z1):
		curx1 = curx + random.random()
		cury1 = cury + random.random()
		curz1 = curz + random.random()

		# this is the perturbed vector to the point
		vec0 = np.array([curx, cury, curz])
		vec1 = np.array([curx1, cury1, curz1])
		vec1 = vec1 / (vec1**2).sum()**.5
		vec1 = vec1 - (vec0 * vec1).sum() *vec0
		vec1 = vec1 / (vec1**2).sum()**.5
		vec2 = np.cross(vec0, vec1)
		# vec0,vec1,vec2 this is the orthogonal system of vectors	

		summer  = lambda t: xpoly * t[0] + ypoly * t[1] + zpoly * t[2]
		norm = summer(vec0)
		curxpoly =  summer(vec1) / norm
		curypoly =  summer(vec2) / norm	
		# curxpoly aren now projections of the polygon vertex onto the plane
		# perpednicular to the vector pointing towards curx,cury,curz
		# the rotation angle of this plane is random 
		
		#print repr(curxpoly),repr(curypoly)
		poly = Polygon(zip(curxpoly, curypoly))
		arr.append(point.within(poly))
	result = int(np.array(arr).sum())
	print >>sys.stderr , result
	ind[ind]=arr[:]
	return result#,ind

def get_rotation_matrix(fi1, fi2, fi3):
	mat1 = np.matrix([
		[ 1, 0,           0            ],
		[ 0, np.cos(fi1), -np.sin(fi1) ],
		[ 0, np.sin(fi1), np.cos(fi1)  ]
		])
	mat2 = np.matrix([
		[ np.cos(fi2),  0, np.sin(fi2) ],
		[ 0,            1, 0           ],
		[ -np.sin(fi2), 0, np.cos(fi2) ]
		])
	mat3 = np.matrix([
		[ np.cos(fi3), -np.sin(fi3), 0 ],
		[ np.sin(fi3), np.cos(fi3),  0 ],
		[ 0,           0,            1 ]
		])
	return mat1*mat2*mat3
		
def gen_random_poly():
	minvert = 3
	maxvert = 10
	mindist = 1
	maxdist = 10
	nvert = np.random.uniform(minvert, maxvert, 1)[0]
	fi1, fi2, fi3 = np.random.uniform(0,2*np.pi,1)[0],\
					np.random.uniform(0,2*np.pi,1)[0],\
					np.random.uniform(0,2*np.pi,1)[0]
	mat = get_rotation_matrix(fi1, fi2, fi3)
	while True:
		ras = np.sort(np.random.uniform(0, 360, nvert))
		if (ras[-1]-ras[0])>180:
			break
	dists = np.random.uniform(mindist, maxdist, nvert)
	decs = (90 - dists)
	#print ras,decs

	x,y,z = torect(ras, decs)
	vec = np.array([x, y, z])
	vec1 = mat * vec
	ras,decs = fromrect(np.array(vec1[0]).flatten(),np.array(vec1[1]).flatten(),
			np.array(vec1[2]).flatten())
	ras = (ras  + (180-ras[0]))%360 - (180-ras[0])
	ras = ras - (ras)%0.001
	ras = (ras+360)%360
	decs = decs - (decs)%0.001
	return ras,decs	
	
def get_all_polys():
	npolys = 1000
	tab = 'test_small'
	np.random.seed(1)
	catra, catdec = readcol('/tmp/zz3_')#./gen_data 3 10000
	pool = multiprocessing.Pool(8)
	res = []
	for i in range(npolys):
		a,b = gen_random_poly()
		coostring = ','.join(['%f'%_ for _ in np.array([a,b]).T.flatten()])

		query = 'select count(*) from %s where q3c_poly_query(ra,dec,ARRAY[%s]);'%(tab,coostring)
		print query
		#print a,b
		#print within_poly(catra,catdec,a,b);
		res.append(pool.apply_async(within_poly,(catra,catdec,a,b)))
	
	pool.close()	
	pool.join()
	for r in res:
		print r.get()