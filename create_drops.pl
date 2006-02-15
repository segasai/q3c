my $state=0;
while(<>)
{
	if (/^--/)
	{
		next;
	}
	elsif (s/CREATE OR REPLACE FUNCTION/DROP FUNCTION/)
	{
		if (!(/\)/))
		{
			$state=1;
			chomp;
			print;
			next;
		}
		chomp;
		print;
		print ";\n";
		next;
	}
	if ($state==1)
	{
		chomp;
		print;
		if (/\)/)
		{
			print ";\n";
			$state=0;
		}
		
	} 

}