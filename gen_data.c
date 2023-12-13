/*
       Copyright (C) 2004-2023 Sergey Koposov

    Email: skoposov AT ed DOT ac DOT uk

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
#include <stdbool.h>
#include <string.h>

static int64_t rand_state = 1;
static const int64_t c = 12345;
static const int64_t m = ((int64_t)1) << 31;
static const int64_t a = 1103515245;

int64_t get_rand()
{
	rand_state = ( a * rand_state + c ) % m;
	return rand_state;
}

/* Run as
 * $ ./gen_data 1 100
 * The first argument of the program is the random seed for the pseudorandom
 * sequence.
 * The second number is the number of objects outputted
 * Additional flags
 * --withpm  output pms as well
 * --pmscale= maximum allowed pm in mas/yr
 * --epoch= epoch of coordinates
 * --randomepoch assign random epoch
 *
 */
/* The random number sequence was based on Knuth's theorem A (from
 * his second book)
 */
int main(int argc, char *argv[])
{
	const int nrabins = 36000;
	const int ndecbins = 18000;
	const int ntotbins = nrabins * ndecbins; /* 2^x*3^y*5^z */
	double corrections[ndecbins], total = 0, pmra, pmdec, pmscale = 1;
	int npoints;
	bool random_epoch = false;
	double epoch, cur_epoch;
	int i, extraarg;
	char parsing_error = 1;
	bool withpm = false;
	// first argument is the seed and then number of points to generate
	if (argc >= 3)
	{
		rand_state = atoi(argv[1]);
		get_rand(); // advance one step
		npoints = atoi(argv[2]);
		parsing_error = 0;
		for (extraarg = 0; extraarg < (argc - 3); extraarg++)
		{
			char *curarg = argv[3 + extraarg];
			if (strncmp(curarg,"--randomepoch", 13) == 0) {random_epoch = true;}
			if (strncmp(curarg,"--withpm", 9) == 0) {withpm = true;}
			if (strncmp(curarg,"--pmscale=", 10) == 0)
			{
				if (sscanf(curarg, "--pmscale=%lf", &pmscale) == 0)
				{
					fprintf(stderr, "Formatting error of pmscale\n");
					exit(1);
				}
			}

			if (strncmp(curarg,"--epoch=",8) == 0)
			{
				sscanf(curarg, "--epoch=%lf", &epoch);
			}
		}
	}

	if (parsing_error)
	{
		fprintf(stderr, "Wrong arguments!\n"
		        "MUST be ./gen_data [RANDOM SEED] [NPOINTS] [PROPERMOTIONSCALE(optional)]");
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
		int64_t ra = (int64_t)(get_rand() * 1. / m * nrabins);
		int64_t dec = (int64_t)(get_rand() * 1. / m * ndecbins);
		if (withpm )
		{
			pmra = ((get_rand() * 1. / m) * 2 - 1) * pmscale;
			pmdec = ((get_rand() * 1. / m) * 2 - 1) * pmscale;
			if (random_epoch)
			{
				cur_epoch = ((get_rand() * 1. / m) ) * 20 + 1980;
			}
			else
			{
				cur_epoch = epoch;
			}
		}

		if (get_rand() < (corrections[dec] * m))
		{
			if (withpm)
			{
				printf("%f %f %f %f %f\n", ra * (360. / nrabins),
				       -90 + dec * (180. / ndecbins), pmra, pmdec, cur_epoch);

			}
			else
			{
				printf("%f %f\n", ra * (360. / nrabins),
				       -90 + dec * (180. / ndecbins));
			}
			npointsleft--;
		}
	}

}
