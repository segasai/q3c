/*
	   Copyright (C) 2004-2011 Sergey Koposov
   
    Author: Sergey Koposov, Sternberg Astronomical Institute
    						Institute of Astronomy, Cambridge
    Email: koposov@ast.cam.ac.uk
    http://lnfm1.sai.msu.ru/~math

    This file is part of Q3C.

    Q3C is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Q3C is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Q3C; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "postgres.h"

#define YN1(a,Yn,m) (a*Yn+1)%m
#define XN(A,Yn,X0,m) (A*Yn+X0)%m

/* Run as 
 * $ ./gen_data 1
 * The number argument of the program is the starting point for the 'random'
 * sequence of numbers. The sequence was based on Knuth's theoreme A (from 
 * the his second book)
 */
int main(int argc, char *argv[])
{
	const int nrabins = 36000; 
	const int ndecbins = 18000;
	const int ntotbins = nrabins * ndecbins; /* 2^x*3^y*5^z */
	double corrections[ndecbins], total = 0;
	int number_array[ndecbins];
	const int npoints = 1000000;
	int npoints0 = 0;
	int i;
	int64 x = 1;
	int64 y = 0;
	if (argc == 2)
	{
		x = atoi(argv[1]);
	}
	else
	{
		exit(1);
	}

	for (i = 0; i < ndecbins; i++)
	{
		corrections[i] = cos((-90. + (180. / ndecbins) * (i + 0.5)) *
			M_PI / 180.);
		total += corrections[i];
	}
	
	for (i = 0; i < ndecbins; i++)
	{
		number_array[i] = npoints * (corrections[i] / total);
		npoints0 += number_array[i];
	}	
	const int64 b = 60 ,c = 7, m = ntotbins;	
	const int64 a = b + 1;
	const int64 X0 = x;
	const int64 A = (X0 * b + c) % m;
	int npoints1 = npoints0;
	while (npoints1)
	{
		x = XN(A, (y = YN1(a,y,m)), X0, m);
		int64 ra = x % nrabins;
		int64 dec = (x - ra) / nrabins;
		if (number_array[dec])
		{
			number_array[dec]--;
			printf("%f %f\n", ra * (360. / nrabins),
				-90 + dec * (180. / ndecbins));
			npoints1--;
		}
	}
	
}
