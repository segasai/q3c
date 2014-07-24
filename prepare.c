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

#include <stdlib.h>
#include <stdio.h>
#include "common.h"
int main()
{
	struct q3c_prm hprm1;
	
	init_q3c1(&hprm1,1073741824);
	
	q3c_dump_prm(&hprm1,"dump.c");
	
	return 0;
	
}
