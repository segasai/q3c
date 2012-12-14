#!/usr/bin/perl

use strict;


# Here the numbers are the number of PAIRS!!! of ranges (as in C program)
my $full_array_len=50;#800;
my $partial_array_len=50;#800;


my $q3c_func='q3c_radial_query_it';

my ($ipix_col, $ra_col, $dec_col, $ra_cen, $dec_cen, $radius, $coltype);
for (1..3)
{
  if ($_==1)
  {
    $ipix_col='$1';
    $ra_col='$2';
    $dec_col='$3';
    $ra_cen='$4';
    $dec_cen='$5';
    $radius='$6';
  print "CREATE OR REPLACE FUNCTION q3c_radial_query(bigint,
                  double precision, double precision,
                  double precision, double precision, double precision)
                                         returns boolean as '";

  }
  else
  {
    $ipix_col='q3c_ang2ipix($1,$2)';
    $ra_col='$1';
    $dec_col='$2';
    $ra_cen='$3';
    $dec_cen='$4';
    $radius='$5';
    if ($_==2)
    {
      $coltype='real';
    }
    else
    {
      $coltype='double precision';    
    }
  print "CREATE OR REPLACE FUNCTION q3c_radial_query(
                  $coltype, $coltype,
                  double precision, double precision, double precision)
                                         returns boolean as '";

  }

  print "SELECT ((\n";

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
  print ") OR (\n";
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

}