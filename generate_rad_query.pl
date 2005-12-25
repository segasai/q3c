#!/usr/bin/perl

use strict;


# Here the numbers are the number of PAIRS!!! of ranges (as in C program)
my $full_array_len=45;#800;
my $partial_array_len=45;#800;


my $ipix_col='$1';

my $q3c_func='q3c_radial_query_it';

my $ra_col='$2';
my $dec_col='$3';
my $ra_cen='$4';
my $dec_cen='$5';
my $radius='$6';

print "CREATE OR REPLACE FUNCTION q3c_radial_query(bigint,
                double precision, double precision,
                double precision, double precision, double precision)
                                       returns boolean as '";
print "SELECT (\n";

my $i = 0;

while(1)
{
  if ($i < 2 * ($full_array_len - 1))
  {
    print "($ipix_col>=$q3c_func($ra_cen,$dec_cen,$radius," . "" . ($i++) . ",1) AND $ipix_col<$q3c_func($ra_cen,$dec_cen,$radius,".($i++).",1)) OR\n";
  }
  else 
  {
    print "($ipix_col>=$q3c_func($ra_cen,$dec_cen,$radius," . "" . ($i++) . ",1) AND $ipix_col<$q3c_func($ra_cen,$dec_cen,$radius,".($i++).",1)) \n";
    last;
  }
}
print ") OR ((\n";
$i=0;

while(1)
{
  if ($i < 2 * ($partial_array_len - 1))
  {
    print "($ipix_col>=$q3c_func($ra_cen,$dec_cen,$radius," . "" . ($i++) . ",0) AND $ipix_col<$q3c_func($ra_cen,$dec_cen,$radius,".($i++).",0)) OR\n";
  }
  else 
  {
    print "($ipix_col>=$q3c_func($ra_cen,$dec_cen,$radius," . "" . ($i++) . ",0) AND $ipix_col<$q3c_func($ra_cen,$dec_cen,$radius,".($i++).",0)) \n";
    last;
  }
}



print ")\n AND q3c_sindist($ra_col,$dec_col,$ra_cen,$dec_cen)<POW(SIN(RADIANS($radius)/2),2)\n)\n";
print "' LANGUAGE SQL IMMUTABLE;";

