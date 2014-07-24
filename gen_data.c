/*
	   Copyright (C) 2004-2014 Sergey Koposov
   
    Email: koposov@ast.cam.ac.uk

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



static int64 rand_state = 1;
static const int64 c = 12345;
static const int64 m = ((int64)1)<<31;	
static const int64 a = 1103515245;

int64 get_rand()
{
	rand_state =  ( a * rand_state + c ) % m;
	return rand_state;
}

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
	int npoints; 
	int i;
	char parsing_error = 1;
	
	// first argument is the seed and then number of points to generate
	if (argc == 3)
	{
		rand_state = atoi(argv[1]);
		get_rand(); // advance one step
		npoints = atoi(argv[2]);
		parsing_error = 0;
	}
	if (parsing_error)
	{
		fprintf(stderr, "Wrong arguments!\n"
						"MUST be ./gen_data [RANDOM SEED] [NPOINTS]");
		exit(1);
	}

	for (i = 0; i < ndecbins; i++)
	/* weights in order to have cosine distribution of declinations 
	 * corresponding to uniform distribution on the sky */
	{
		corrections[i] = cos((-90. + (180. / ndecbins) * (i + 0.5)) *
			M_PI / 180.);
	}
	
	int npointsleft = npoints;
	while (npointsleft)
	{
		int64 ra = (int64)(get_rand() * 1./m * nrabins);
		int64 dec = (int64)(get_rand() * 1./m * ndecbins);
		
		if (get_rand() < (corrections[dec]*m))
		{
			printf("%f %f\n", ra * (360. / nrabins),
				-90 + dec * (180. / ndecbins));
			npointsleft--;
		}
	}
	
}
