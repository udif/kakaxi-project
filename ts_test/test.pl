
use strict;
use warnings;
use threads;
use Thread::Semaphore;
use Cwd 'abs_path'; 
#print abs_path($0); 
#my $rv = system("tsudpsend.exe xuanchuan.ts 192.168.2.100 49212 1 3700000");
#my $rv = system("sbbt_produce.exe 666000 6875 64 192.168.3.100 49212 5000 1 4990");
#print $rv;
#sleep 5;

=pod
my $pid=fork();
if($pid==0)
{
   exec("tsudpsend.exe xuanchuan.ts 192.168.2.100 49212 1 3700000");
   exit(0);
}
print("pid1 = $pid\n");
sleep 5;
kill('KILL', $pid);
=cut
system("a.exe xuanchuan.ts 192.168.2.100 49156 1 5000000 1000");
print "ok";














=pod
my $abs_file = abs_path($0);
my $back_file = "$abs_file";
my $flag_file = $back_file.'___flag';
open F,">$flag_file";
close F;
#my $dirname = dirname($back_file);
#system("mkdir -p $dirname") unless(-e $dirname);
#system("cp -f $abs_file $back_file");
#unlink $flag_file;




use warnings;  
use strict;  

my $ip = "192.168.2.2";
my $maske = "255.255.255.0";

my @ip = split (/\./,$ip);
my @mask = split(/\./,$maske);
my $bitsInNetmask = 0;
foreach my $a ( @mask ) {
	$bitsInNetmask +=
	($a == 255) ? 8 :
	($a == 254) ? 7 :
	($a == 252) ? 6 :
	($a == 248) ? 5 :
	($a == 240) ? 4 :
	($a == 224) ? 3 :
	($a == 192) ? 2 :
	($a == 128) ? 1 :
	($a == 0) ? 0 :
			die("Invalid netmask");
}
for my $i (0..3){
	$ip[$i] = int($ip[$i]) & int($mask[$i]);
}
print ($ip[0] . "." . $ip[1] . "." . $ip[2] . "." . $ip[3] . "/" . $bitsInNetmask);



sub cidr_ip{
	my @ip = split (/\./,shift);
    my @mask = split(/\./,shift);
	my $bitsInNetmask = 0;
	foreach my $a ( @mask ) {
		$bitsInNetmask +=
		($a == 255) ? 8 :
		($a == 254) ? 7 :
		($a == 252) ? 6 :
		($a == 248) ? 5 :
		($a == 240) ? 4 :
		($a == 224) ? 3 :
		($a == 192) ? 2 :
		($a == 128) ? 1 :
		($a == 0) ? 0 : 0;
				#die("Invalid netmask");
	}
	for my $i (0..3){
	    $ip[$i] = int($ip[$i]) & int($mask[$i]);
    }
	return ($ip[0] . "." . $ip[1] . "." . $ip[2] . "." . $ip[3] . "/" . $bitsInNetmask);
}

=cut
