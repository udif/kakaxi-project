
use strict;
use warnings;
use File::Basename;

use Win32::GUI;
use Win32 ();
use Win32::GUI::Loft;

use threads;
use threads::shared;
use Thread::Semaphore;

use Data::Dumper;

use Net::Pcap;
use Net::PcapUtils;
BEGIN{
	push @INC,dirname($0);
}
use ini;

my $main;			##GUI对象
my $tested_cmc;		##cmc对象
my $sem_main:shared;
my @devs:shared;
my $err = '';
my $buttonx = 0;

my $local_mac = "50 44 33 22 11 ee ";
my $remote_mac = "52 54 4f 11 22 44 ";
my $opt_code = "88 a8 08 01 ";
my $tpacket = "08 00 45 00 00 3c 50 5d 00 00 80 01 0d a5 0a 07 64 b0 0a 07 64 01 08 00 20 5c 04 00 29 00 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 61 62 63 64 65 66 67 68 69";
my $test_package = "";

my @test_packet;
#$test_packet[0] = "52 54 4f 11 22 33 52 54 4f 11 22 44 88 a8 08 01 08 00 45 00 00 3c 50 5d 00 00 80 01 0d a5 0a 07 64 b0 0a 07 64 01 08 00 20 5c 04 00 29 00 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 61 62 63 64 65 66 67 68 69";
#$test_packet[0] = "00 10 18 ea 00 43 00 e0 4c 1f 00 b0 08 00 45 00 00 54 f7 4c 40 00 40 01 bd 37 c0 a8 02 6f c0 a8 02 65 00 00 7b 78 01 0d 00 03 00 00 00 89 00 03 82 eb 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";
$test_packet[0] = "50 44 33 22 11 ee 52 54 4f 11 22 44 88 a8 08 01 08 00 45 00 00 3c 50 5d 00 00 80 01 0d a5 0a 07 64 b0 0a 07 64 01 08 00 20 5c 04 00 29 00 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 61 62 63 64 65 66 67 68 69";

$test_packet[1] = "52 54 4f 11 22 44 52 54 4f 11 22 33 08 00 45 00 00 3c 50 5d 00 00 80 01 0d a5 0a 07 64 b0 0a 07 64 01 08 00 20 5c 04 00 29 00 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 61 62 63 64 65 66 67 68 69";



##开始运行程序
ipqam_test_main();

sub ipqam_test_main{
	##-------------初始化gui，cmc， ---------------##


	##创建GUI对象
	my $win_file=dirname($0)."/main.gld";
	my $objDesign = Win32::GUI::Loft::Design->newLoad($win_file) or die("Could not open window file ($win_file)");
	$main = $objDesign->buildWindow( undef ) or die("Could not build window ($win_file)");
	gui_onload();

	##创建一个线程开始常规测试
	$sem_main = Thread::Semaphore->new(0);
	my $thread_pid = threads->new ( \&main_test,);

	##运行对话框程序
	$main->Show();
	Win32::GUI::Dialog();
}

sub gui_onload{
	
	my %devinfo;
	@devs = Net::Pcap::findalldevs(\%devinfo, \$err);
	for my $dev (@devs){
		print "$dev : $devinfo{$dev}\n";
	}
	$main->cbDev0->Add(map{$devinfo{$_}} @devs);
	$main->cbDev0->SelectString($devinfo{$devs[0]});
	$main->cbDev1->Add(map{$devinfo{$_}} @devs);
	$main->cbDev1->SelectString($devinfo{$devs[0]});
	$main->tfSend0->Text($test_packet[0]);
	$main->tfSend1->Text($test_packet[1]);
	$main->lbl_clash_mac->Text($remote_mac);
}

sub main_test{
	$sem_main->down();
	send_packet($buttonx);
}


sub send_packet{
	my $id = shift;
	my $dev = $devs[$main->{"cbDev$id"}->GetCurSel()];
	my $pcap = Net::Pcap::open_live($dev, 1524, 1, 10, \$err) or die "$! $err";

	my $packet;
	srand;
	
	creat_file();
	my $mac_file=dirname($0)."/mac.conf";
	my $new_cfg = ini::ini_parser_file($mac_file);
		
	if($main->rb_once->Checked()){
		#loop_y();
		foreach my $n (0..($main->tf_send_times->Text()-1)){
			my $num;
			if($main->rb_random->Checked()){
				$num = int(rand($new_cfg->{global}{mac_num}));
			}else{
				if($n < $new_cfg->{global}{mac_num}){
					$num = $n;
				}else{
					$num = ($n)%($new_cfg->{global}{mac_num});
				}
			}
			my $str = $new_cfg->{global}{"mac_$num"};
			my $my_mac = substr($str,0,2) . " " . substr($str,2,2) . " " . substr($str,4,2) . " " . substr($str,6,2) . " " . substr($str,8,2) . " " . substr($str,10,2) . " ";
			my $my_packet = $my_mac . " " . $my_mac . " " . $opt_code . $tpacket;
			$packet = pack("C*", map{hex} split(/\s+/, $my_packet));
			Net::Pcap::sendpacket($pcap, $packet);
		}
		print "send packet ok";
	} else {
		#substr($str,0,2);
		my $num;
		while(1){
			if($main->rb_random->Checked()){
				$num = int(rand($new_cfg->{global}{mac_num}));
			}
			my $str = $new_cfg->{global}{"mac_$num"};
			my $my_mac = substr($str,0,2) . " " . substr($str,2,2) . " " . substr($str,4,2) . " " . substr($str,6,2) . " " . substr($str,8,2) . " " . substr($str,10,2) . " ";
			
			my $my_packet = $my_mac . " " . $my_mac . " " . $opt_code . $tpacket;
			$packet = pack("C*", map{hex} split(/\s+/, $my_packet));
			Net::Pcap::sendpacket($pcap, $packet);
			$num++;
			if($num > 99999){
				$num = 0;
			}
		}
		
	}
}




sub generate_pack{
	my $id = shift;

	$test_packet[0] = $local_mac . $remote_mac . $opt_code . $tpacket;
	$test_packet[1] = $remote_mac . $local_mac . $opt_code . $tpacket;
}


sub change_hex{
	my $a = shift;
	if($a <= 9){
		return $a;
	} elsif($a == 10){
		return "a";
	} elsif($a == 11){
		return "b";
	} elsif($a == 12){
		return "c";
	} elsif($a == 13){
		return "d";
	} elsif($a == 14){
		return "e";
	} elsif($a == 15){
		return "f";
	}
}


sub loop_y{
	my @loop_bit = (4,4,2,2,1,1,0xf,4,4,5,2,5);
	
	my $mac_file=dirname($0)."/mac.conf";
	open MAC_TABLE,">$mac_file";
	binmode MAC_TABLE;
	my $head = <<HEAD;
[global]
HEAD
	
	foreach my $k (0..11){
		if($main->{"chb$k"}->Checked()){
			$loop_bit[$k] = 0;
		}
	}
	
	for(;;){#$loop_bit[11] < 16
		$head .= creat_mac($loop_bit[0],$loop_bit[1],$loop_bit[2],$loop_bit[3],$loop_bit[4],$loop_bit[5],$loop_bit[6],$loop_bit[7],$loop_bit[8],$loop_bit[9],$loop_bit[10],$loop_bit[11]);
		
		if($main->{chb0}->Checked()&&($loop_bit[0] < 15)){
			$loop_bit[0]++;
		}elsif($main->{chb1}->Checked()&&($loop_bit[1] < 15)){
			$loop_bit[1]++;
			$loop_bit[0] = 0 if($main->{chb0}->Checked());
		}elsif($main->{chb2}->Checked()&&($loop_bit[2] < 15)){
			$loop_bit[2]++;
			$loop_bit[0] = 0 if($main->{chb0}->Checked());
			$loop_bit[1] = 0 if($main->{chb1}->Checked());
		}elsif($main->{chb3}->Checked()&&($loop_bit[3] < 15)){
			$loop_bit[3]++;
			$loop_bit[0] = 0 if($main->{chb0}->Checked());
			$loop_bit[1] = 0 if($main->{chb1}->Checked());
			$loop_bit[2] = 0 if($main->{chb2}->Checked());
		}elsif($main->{chb4}->Checked()&&($loop_bit[4] < 15)){
			$loop_bit[4]++;
			$loop_bit[0] = 0 if($main->{chb0}->Checked());
			$loop_bit[1] = 0 if($main->{chb1}->Checked());
			$loop_bit[2] = 0 if($main->{chb2}->Checked());
			$loop_bit[3] = 0 if($main->{chb3}->Checked());
		}elsif($main->{chb5}->Checked()&&($loop_bit[5] < 15)){
			$loop_bit[5]++;
			$loop_bit[0] = 0 if($main->{chb0}->Checked());
			$loop_bit[1] = 0 if($main->{chb1}->Checked());
			$loop_bit[2] = 0 if($main->{chb2}->Checked());
			$loop_bit[3] = 0 if($main->{chb3}->Checked());
			$loop_bit[4] = 0 if($main->{chb4}->Checked());
		}elsif($main->{chb6}->Checked()&&($loop_bit[6] < 15)){
			$loop_bit[6]++;
			$loop_bit[0] = 0 if($main->{chb0}->Checked());
			$loop_bit[1] = 0 if($main->{chb1}->Checked());
			$loop_bit[2] = 0 if($main->{chb2}->Checked());
			$loop_bit[3] = 0 if($main->{chb3}->Checked());
			$loop_bit[4] = 0 if($main->{chb4}->Checked());
			$loop_bit[5] = 0 if($main->{chb5}->Checked());
		}elsif($main->{chb7}->Checked()&&($loop_bit[7] < 15)){
			$loop_bit[7]++;
			$loop_bit[0] = 0 if($main->{chb0}->Checked());
			$loop_bit[1] = 0 if($main->{chb1}->Checked());
			$loop_bit[2] = 0 if($main->{chb2}->Checked());
			$loop_bit[3] = 0 if($main->{chb3}->Checked());
			$loop_bit[4] = 0 if($main->{chb4}->Checked());
			$loop_bit[5] = 0 if($main->{chb5}->Checked());
			$loop_bit[6] = 0 if($main->{chb6}->Checked());
		}elsif($main->{chb8}->Checked()&&($loop_bit[8] < 15)){
			$loop_bit[8]++;
			$loop_bit[0] = 0 if($main->{chb0}->Checked());
			$loop_bit[1] = 0 if($main->{chb1}->Checked());
			$loop_bit[2] = 0 if($main->{chb2}->Checked());
			$loop_bit[3] = 0 if($main->{chb3}->Checked());
			$loop_bit[4] = 0 if($main->{chb4}->Checked());
			$loop_bit[5] = 0 if($main->{chb5}->Checked());
			$loop_bit[6] = 0 if($main->{chb6}->Checked());
			$loop_bit[7] = 0 if($main->{chb7}->Checked());
		}elsif($main->{chb9}->Checked()&&($loop_bit[9] < 15)){
			$loop_bit[9]++;
			$loop_bit[0] = 0 if($main->{chb0}->Checked());
			$loop_bit[1] = 0 if($main->{chb1}->Checked());
			$loop_bit[2] = 0 if($main->{chb2}->Checked());
			$loop_bit[3] = 0 if($main->{chb3}->Checked());
			$loop_bit[4] = 0 if($main->{chb4}->Checked());
			$loop_bit[5] = 0 if($main->{chb5}->Checked());
			$loop_bit[6] = 0 if($main->{chb6}->Checked());
			$loop_bit[7] = 0 if($main->{chb7}->Checked());
			$loop_bit[8] = 0 if($main->{chb8}->Checked());
		}elsif($main->{chb10}->Checked()&&($loop_bit[10] < 15)){
			$loop_bit[10]++;
			$loop_bit[0] = 0 if($main->{chb0}->Checked());
			$loop_bit[1] = 0 if($main->{chb1}->Checked());
			$loop_bit[2] = 0 if($main->{chb2}->Checked());
			$loop_bit[3] = 0 if($main->{chb3}->Checked());
			$loop_bit[4] = 0 if($main->{chb4}->Checked());
			$loop_bit[5] = 0 if($main->{chb5}->Checked());
			$loop_bit[6] = 0 if($main->{chb6}->Checked());
			$loop_bit[7] = 0 if($main->{chb7}->Checked());
			$loop_bit[8] = 0 if($main->{chb8}->Checked());
			$loop_bit[9] = 0 if($main->{chb9}->Checked());
		}elsif($main->{chb11}->Checked()&&($loop_bit[11] < 15)){
			$loop_bit[11]++;
			$loop_bit[0] = 0 if($main->{chb0}->Checked());
			$loop_bit[1] = 0 if($main->{chb1}->Checked());
			$loop_bit[2] = 0 if($main->{chb2}->Checked());
			$loop_bit[3] = 0 if($main->{chb3}->Checked());
			$loop_bit[4] = 0 if($main->{chb4}->Checked());
			$loop_bit[5] = 0 if($main->{chb5}->Checked());
			$loop_bit[6] = 0 if($main->{chb6}->Checked());
			$loop_bit[7] = 0 if($main->{chb7}->Checked());
			$loop_bit[8] = 0 if($main->{chb8}->Checked());
			$loop_bit[9] = 0 if($main->{chb9}->Checked());
			$loop_bit[10] = 0 if($main->{chb10}->Checked());
		}else{
			last;
		}
		
	}
	print MAC_TABLE $head;
	close MAC_TABLE;
}



sub change_pack{
	my $change_mac = shift;

	my $a4 = sprintf("%02x", $change_mac&0xff);
	my $a5 = sprintf("%02x", ($change_mac>>8)&0xff);
	my $a6 = sprintf("%02x", ($change_mac>>16)&0xff);
	my $a7 = sprintf("%02x", ($change_mac>>24)&0xff);

	my $aa = $a7 . " " . $a6 . " " . $a5 . " " . $a4 . " 22 44 \n"; 
	#my $my_packet = $local_mac . $aa . $opt_code . $tpacket;
	return $aa;
}

sub creat_mac{
	my $a0 = shift;
	my $a1 = shift;
	my $a2 = shift;
	my $a3 = shift;
	my $a4 = shift;
	my $a5 = shift;
	my $a6 = shift;
	my $a7 = shift;
	my $a8 = shift;
	my $a9 = shift;
	my $a10 = shift;
	my $a11 = shift;
	return change_hex($a11) . change_hex($a10) . " " . change_hex($a9) . change_hex($a8) . " " . change_hex($a7) . change_hex($a6) . " " . change_hex($a5) . change_hex($a4) . " " . change_hex($a3) . change_hex($a2) . " " . change_hex($a1) . change_hex($a0) . "\n";
}


#

sub creat_file{
	my $mac_file=dirname($0)."/mac.conf";
	open MAC_TABLE,">$mac_file";
	binmode MAC_TABLE;
	my $head = <<HEAD;
[global]
HEAD

	foreach my $m (0..99999){
		$head .= "mac_$m = ";
		foreach my $n (0..11){
			if($main->{"chb$n"}->Checked()){
				$head .= change_hex(int(rand(16)));
			}else{
				$head .= change_hex(14);
			}
		}
		$head .= "\n";
	}
	$head .= "mac_num = 100000\n";
	print MAC_TABLE $head;
	close MAC_TABLE;
	print "creat mac file ok\n";
}


sub btnSend0_Click{
	$buttonx = 0;
	$sem_main->up();
}

sub btnSend1_Click{
	$buttonx = 1;
	$sem_main->up();
}


=pod	
	foreach my $n (0..11){
		if($main->{"chb$n"}->Checked()){
			$head .= (int(rand(16)));
		}
	}
	
	#foreach my $m (0..1){
		my $t0 = change_hex($main->{chb0}->Checked()?int(rand(16)):4);
		my $t1 = change_hex($main->{chb1}->Checked()?int(rand(16)):4);
		my $t2 = change_hex($main->{chb2}->Checked()?int(rand(16)):2);
		my $t3 = change_hex($main->{chb3}->Checked()?int(rand(16)):2);
		my $t4 = change_hex($main->{chb4}->Checked()?int(rand(16)):1);
		my $t5 = change_hex($main->{chb5}->Checked()?int(rand(16)):1);
		my $t6 = change_hex($main->{chb6}->Checked()?int(rand(16)):15);
		my $t7 = change_hex($main->{chb7}->Checked()?int(rand(16)):4);
		my $t8 = change_hex($main->{chb8}->Checked()?int(rand(16)):4);
		my $t9 = change_hex($main->{chb9}->Checked()?int(rand(16)):5);
		my $t10 = change_hex($main->{chb10}->Checked()?int(rand(16)):2);
		my $t11 = change_hex($main->{chb11}->Checked()?int(rand(16)):5);
	
		#$head .= "mac_ = ".$t11.$t10." ".$t9.$t8." ".$t7.$t6." ".$t5.$t4." ".$t3.$t2." ".$t1.$t0."\n";
	#}
=cut