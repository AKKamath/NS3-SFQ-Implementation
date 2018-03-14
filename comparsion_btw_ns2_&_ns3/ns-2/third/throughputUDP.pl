# type: perl throughput.pl <trace file> <required node> <granlarity>   >    output file

$infile = $ARGV[0];
$tonode = $ARGV[1];
$granularity = $ARGV[2];

#we compute how many bytes were transmitted during time interval specified
#by granularity parameter in seconds
$sum = 0;
$clock = 0;

open (DATA,"<$infile")
|| die "Can't open $infile $!";

while (<DATA>) {
	@x = split(' ');

	if ($x[0] eq 'r') 
	{ 
		#checking if the destination corresponds to arg 1
		if ($x[3] eq $tonode && $x[4] eq 'cbr') 
		{ 
			#checking if the packet type is TCP
			$sum=$sum+$x[5];
		}
	}

	#column 1 is time 
	if ($x[1]-$clock > $granularity)
	{
		$throughput = $sum *8.0/ (($x[1] - $clock)*1000*1000);
		print STDOUT "$x[1] $throughput\n";
		$clock = $x[1];
		$sum = 0;
	}
}


$throughput = $sum / ($x[1] - $clock);
print STDOUT "$x[1] $throughput\n";
$clock = $x[1];
$sum = 0;


close DATA;
exit(0);
