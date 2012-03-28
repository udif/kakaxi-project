
use strict;
use warnings;
use File::Basename;

use Win32::GUI;
use Win32 ();
use Win32::GUI::Loft;

use threads;
use Data::Dumper;

use Net::Pcap;
use Net::PcapUtils;
BEGIN{
	push @INC,dirname($0);
}
use ini;

my $win_file=dirname($0)."/main.gld";
my $objDesign = Win32::GUI::Loft::Design->newLoad($win_file) or die("Could not open window ($win_file)");
my $main = $objDesign->buildWindow(undef) or die("Could not build window ($win_file)");


my $err = '';
my %devinfo;
my @devs = Net::Pcap::findalldevs(\%devinfo, \$err);
for my $dev (@devs){
	print "$dev : $devinfo{$dev}\n";
}

$main->cbDev0->Add(map{$devinfo{$_}} @devs);
$main->cbDev0->SelectString($devinfo{$devs[0]});
$main->cbDev1->Add(map{$devinfo{$_}} @devs);
$main->cbDev1->SelectString($devinfo{$devs[0]});

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


$main->tfSend0->Text($test_packet[0]);
$main->tfSend1->Text($test_packet[1]);
$main->lbl_clash_mac->Text($remote_mac);

$main->Show();
Win32::GUI::Dialog();

sub send_packet{
	my $id = shift;
	my $dev = $devs[$main->{"cbDev$id"}->GetCurSel()];
	my $pcap = Net::Pcap::open_live($dev, 1524, 1, 10, \$err) or die "$! $err";

	my $packet;
	srand;
	loop_y();
	my $mac_file=dirname($0)."/mac.conf";
	my $new_cfg = ini::ini_parser_file($mac_file);
		
	if($main->rb_once->Checked()){
		
		foreach my $n (0..($main->tf_send_times->Text()-1)){
			my $num;
			if($main->rb_random->Checked()){
				
				$num = int(rand($new_cfg->{global}{mac_num}));
				print $num . "\n";
			}else{
				if($n < $new_cfg->{global}{mac_num}){
					$num = $n;
				}else{
					$num = ($n)%($new_cfg->{global}{mac_num});
				}
			}
			my $my_packet = $new_cfg->{global}{"mac_$num"} . " " . $opt_code . $tpacket;
			$packet = pack("C*", map{hex} split(/\s+/, $my_packet));
			Net::Pcap::sendpacket($pcap, $packet);
		}
	} else {
		#generate_clash_mac();
		#loop_x($id);
		
		my $lop = 0;
		while(1){
			my $num;
			if($main->rb_random->Checked()){
				$num = int(rand($new_cfg->{global}{mac_num}));
				print $num . "\n";
			}else{
				if($lop < $new_cfg->{global}{mac_num}){
					$num = $lop;
				}else{
					$num = ($lop)%($new_cfg->{global}{mac_num});
				}
			}
			my $my_packet = $new_cfg->{global}{"mac_$num"} . " " . $opt_code . $tpacket;
			$packet = pack("C*", map{hex} split(/\s+/, $my_packet));
			Net::Pcap::sendpacket($pcap, $packet);
			$lop++;
		}
	}
}

sub btnSend0_Click{
		generate_pack(0);
		send_packet(0);

	0;
}

sub btnSend1_Click{
#	while(1){
		generate_pack(1);
		send_packet(1);
#	}
	0;
}

sub generate_pack{
	my $id = shift;

	$test_packet[0] = $local_mac . $remote_mac . $opt_code . $tpacket;
	$test_packet[1] = $remote_mac . $local_mac . $opt_code . $tpacket;
}

sub generate_clash_mac{
	if($main->chb_clash->Checked()){
		my @change_mac = (4,4,2,2,1,1,0xf,4,4,5,2,5);
		my $clash_mac = 0;
		my $ping_mac;
		
		foreach my $k (0..11){
			$change_mac[$k] = 0 if($main->{"chb$k"}->Checked());
		}
		
		foreach my $j (0..11){
			if($main->{"chb$j"}->Checked()){
				foreach my $l (0..15){
					last if($clash_mac >= $main->tf_clash_times->Text());
					$change_mac[$j] = $l;
					#print "----$change_mac[$j]";
					$ping_mac .= "mac_$clash_mac = " . change_hex($change_mac[11]) . change_hex($change_mac[10]) . " " . change_hex($change_mac[9]) . change_hex($change_mac[8]) . " " . change_hex($change_mac[7]) . change_hex($change_mac[6]) . " " . change_hex($change_mac[5]) . change_hex($change_mac[4]) . " " . change_hex($change_mac[3]) . change_hex($change_mac[2]) . " " . change_hex($change_mac[1]) . change_hex($change_mac[0]) . " \n";
					$clash_mac++;
					$change_mac[$j] = 0;
				}
				print "$ping_mac\n";
			}
		
			#foreach my $i (1..$main->tf_clash_times->Text()){
				#$head .= "mac_${i} = $mac";
			#}
		}
		#print MAC_TABLE $head;
	}
}




sub change_hex{
	my $a = shift;
	if(($a >= 0)&&($a <= 9)){
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


sub loop_x{
	my $dev = shift;
	my $loop_bit = 0x52_54_4f_11;
	my $data16;
	my $total_send = 0;
	
	foreach my $k (4..11){
		if($main->{"chb$k"}->Checked()){
			$loop_bit &=  (~(0xf<<(($k-4)*4)));
		}
	}
	$total_send++;
	#$data16=sprintf("%x", $loop_bit); 
	#print "$data16\n";	
	send_pack($loop_bit, $dev);
	
	for(my $a = $loop_bit; $a < 0xffffffff;){
		if($main->{chb4}->Checked()&&(($a&0xf) != 0xf)){
			$a++;
			$total_send++;
			#$data16=sprintf("%x", $a); 
			#print "$data16\n";	
			send_pack($a, $dev);
		}elsif($main->{chb5}->Checked()&&(($a&0xf0) != 0xf0)){
			$a += 0x10;
			$total_send++;
			if($main->{chb4}->Checked()){
				$a &= 0xfffffff0;	
			}
			#$data16=sprintf("%x", $a); 
			#print "$data16\n";
			send_pack($a, $dev);
		}elsif($main->{chb6}->Checked()&&(($a&0xf00) != 0xf00)){
			$a += 0x100;
			$total_send++;
			if(($main->{chb4}->Checked()) or ($main->{chb5}->Checked())){
				$a &= 0xfffffff0 if($main->{chb4}->Checked());
				$a &= 0xffffff0f if($main->{chb5}->Checked());
				
			}
			#$data16=sprintf("%x", $a); 
			#print "$data16\n";	
			send_pack($a, $dev);
		}elsif($main->{chb7}->Checked()&&(($a&0xf000) != 0xf000)){
			$a += 0x1000;
			$total_send++;
			if($main->{chb4}->Checked() or $main->{chb5}->Checked() or $main->{chb6}->Checked()){
				$a &= 0xfffffff0 if($main->{chb4}->Checked());
				$a &= 0xffffff0f if($main->{chb5}->Checked());
				$a &= 0xfffff0ff if($main->{chb6}->Checked());	
			}
			#$data16=sprintf("%x", $a); 
			#print "$data16\n";
			send_pack($a, $dev);
		}elsif($main->{chb8}->Checked()&&(($a&0xf0000) != 0xf0000)){
			$a += 0x10000;
			$total_send++;
			if($main->{chb4}->Checked() or $main->{chb5}->Checked() or $main->{chb6}->Checked() or $main->{chb7}->Checked()){
				$a &= 0xfffffff0 if($main->{chb4}->Checked());
				$a &= 0xffffff0f if($main->{chb5}->Checked());
				$a &= 0xfffff0ff if($main->{chb6}->Checked());
				$a &= 0xffff0fff if($main->{chb7}->Checked());	
			}
			#$data16=sprintf("%x", $a); 
			send_pack($a, $dev);
			#print "$data16\n";
		}elsif($main->{chb9}->Checked()&&(($a&0xf00000) != 0xf00000)){
			$a += 0x100000;
			$total_send++;
			if($main->{chb4}->Checked() or $main->{chb5}->Checked() or $main->{chb6}->Checked() or $main->{chb7}->Checked() or $main->{chb8}->Checked()){
				$a &= 0xfffffff0 if($main->{chb4}->Checked());
				$a &= 0xffffff0f if($main->{chb5}->Checked());
				$a &= 0xfffff0ff if($main->{chb6}->Checked());
				$a &= 0xffff0fff if($main->{chb7}->Checked());
				$a &= 0xfff0ffff if($main->{chb8}->Checked());	
			}
			#$data16=sprintf("%x", $a); 
			#print "$data16\n";
			send_pack($a, $dev);
		}elsif($main->{chb10}->Checked()&&(($a&0xf000000) != 0xf000000)){
			$a += 0x1000000;
			$total_send++;
			if($main->{chb4}->Checked() or $main->{chb5}->Checked() or $main->{chb6}->Checked() or $main->{chb7}->Checked() or $main->{chb8}->Checked() or $main->{chb9}->Checked()){
				$a &= 0xfffffff0 if($main->{chb4}->Checked());
				$a &= 0xffffff0f if($main->{chb5}->Checked());
				$a &= 0xfffff0ff if($main->{chb6}->Checked());
				$a &= 0xffff0fff if($main->{chb7}->Checked());
				$a &= 0xfff0ffff if($main->{chb8}->Checked());
				$a &= 0xff0fffff if($main->{chb9}->Checked());	
			}
			#$data16=sprintf("%x", $a); 
			#print "$data16\n";
			send_pack($a, $dev);
		}elsif($main->{chb11}->Checked()&&(($a&0xf0000000) != 0xf0000000)){
			$a += 0x10000000;
			$total_send++;
			if($main->{chb4}->Checked() or $main->{chb5}->Checked() or $main->{chb6}->Checked() or $main->{chb7}->Checked() or $main->{chb8}->Checked() or $main->{chb9}->Checked() or $main->{chb10}->Checked()){
				$a &= 0xfffffff0 if($main->{chb4}->Checked());
				$a &= 0xffffff0f if($main->{chb5}->Checked());
				$a &= 0xfffff0ff if($main->{chb6}->Checked());
				$a &= 0xffff0fff if($main->{chb7}->Checked());
				$a &= 0xfff0ffff if($main->{chb8}->Checked());
				$a &= 0xff0fffff if($main->{chb9}->Checked());
				$a &= 0xf0ffffff if($main->{chb10}->Checked());	
			}
			#$data16=sprintf("%x", $a); 
			#print "$data16\n";
			send_pack($a, $dev);
		}else{
			last;
		}
	}
}


sub send_pack{
	my $change_mac = shift;
	my $id = shift;
	my $dev = $devs[$main->{"cbDev$id"}->GetCurSel()];
	my $pcap = Net::Pcap::open_live($dev, 1524, 1, 10, \$err) or die "$! $err";
	
	my $data16=sprintf("%x", $change_mac);
	my $a4 = sprintf("%x", $change_mac&0xff);
	my $a5 = sprintf("%x", ($change_mac>>8)&0xff);
	my $a6 = sprintf("%x", ($change_mac>>16)&0xff);
	my $a7 = sprintf("%x", ($change_mac>>24)&0xff);

	my $aa = $a7 . " " . $a6 . " " . $a5 . " " . $a4; 
	my $my_packet = $local_mac . $aa . " 22 44 " . $opt_code . $tpacket;
	
	my $packet = pack("C*", map{hex} split(/\s+/, $my_packet));
	Net::Pcap::sendpacket($pcap, $packet);
}


sub loop_y{
	my $loop_bit = 0x52_54_4f_11;
	my $data16;
	my $total_send = 0;
	
	my $mac_file=dirname($0)."/mac.conf";
	open MAC_TABLE,">$mac_file";
	binmode MAC_TABLE;
	my $head = <<HEAD;
[global]
HEAD
	
	foreach my $k (4..11){
		if($main->{"chb$k"}->Checked()){
			$loop_bit &=  (~(0xf<<(($k-4)*4)));
		}
	}
	
	$head .= "mac_${total_send} = " . change_pack($loop_bit);
	$total_send++;
	
	for(my $a = $loop_bit; $a < 0xffffffff;){
		if($main->{chb4}->Checked()&&(($a&0xf) != 0xf)){
			$a++;
			
			#send_pack($a, $dev);
			$head .= "mac_${total_send} = " . change_pack($a);
			$total_send++;
		}elsif($main->{chb5}->Checked()&&(($a&0xf0) != 0xf0)){
			$a += 0x10;
			if($main->{chb4}->Checked()){
				$a &= 0xfffffff0;	
			}
			$head .= "mac_${total_send} = " . change_pack($a);
			$total_send++;
			
		}elsif($main->{chb6}->Checked()&&(($a&0xf00) != 0xf00)){
			$a += 0x100;
			
			if(($main->{chb4}->Checked()) or ($main->{chb5}->Checked())){
				$a &= 0xfffffff0 if($main->{chb4}->Checked());
				$a &= 0xffffff0f if($main->{chb5}->Checked());
				
			}
			$head .= "mac_${total_send} = " . change_pack($a);
			$total_send++;
		}elsif($main->{chb7}->Checked()&&(($a&0xf000) != 0xf000)){
			$a += 0x1000;
			
			if($main->{chb4}->Checked() or $main->{chb5}->Checked() or $main->{chb6}->Checked()){
				$a &= 0xfffffff0 if($main->{chb4}->Checked());
				$a &= 0xffffff0f if($main->{chb5}->Checked());
				$a &= 0xfffff0ff if($main->{chb6}->Checked());	
			}
			$head .= "mac_${total_send} = " . change_pack($a);
			$total_send++;
			
		}elsif($main->{chb8}->Checked()&&(($a&0xf0000) != 0xf0000)){
			$a += 0x10000;
			
			if($main->{chb4}->Checked() or $main->{chb5}->Checked() or $main->{chb6}->Checked() or $main->{chb7}->Checked()){
				$a &= 0xfffffff0 if($main->{chb4}->Checked());
				$a &= 0xffffff0f if($main->{chb5}->Checked());
				$a &= 0xfffff0ff if($main->{chb6}->Checked());
				$a &= 0xffff0fff if($main->{chb7}->Checked());	
			}
			$head .= "mac_${total_send} = " . change_pack($a);
			$total_send++;
		}elsif($main->{chb9}->Checked()&&(($a&0xf00000) != 0xf00000)){
			$a += 0x100000;
			
			if($main->{chb4}->Checked() or $main->{chb5}->Checked() or $main->{chb6}->Checked() or $main->{chb7}->Checked() or $main->{chb8}->Checked()){
				$a &= 0xfffffff0 if($main->{chb4}->Checked());
				$a &= 0xffffff0f if($main->{chb5}->Checked());
				$a &= 0xfffff0ff if($main->{chb6}->Checked());
				$a &= 0xffff0fff if($main->{chb7}->Checked());
				$a &= 0xfff0ffff if($main->{chb8}->Checked());	
			}
			$head .= "mac_${total_send} = " . change_pack($a);
			$total_send++;
		}elsif($main->{chb10}->Checked()&&(($a&0xf000000) != 0xf000000)){
			$a += 0x1000000;
			
			if($main->{chb4}->Checked() or $main->{chb5}->Checked() or $main->{chb6}->Checked() or $main->{chb7}->Checked() or $main->{chb8}->Checked() or $main->{chb9}->Checked()){
				$a &= 0xfffffff0 if($main->{chb4}->Checked());
				$a &= 0xffffff0f if($main->{chb5}->Checked());
				$a &= 0xfffff0ff if($main->{chb6}->Checked());
				$a &= 0xffff0fff if($main->{chb7}->Checked());
				$a &= 0xfff0ffff if($main->{chb8}->Checked());
				$a &= 0xff0fffff if($main->{chb9}->Checked());	
			}
			$head .= "mac_${total_send} = " . change_pack($a);
			$total_send++;
		}elsif($main->{chb11}->Checked()&&(($a&0xf0000000) != 0xf0000000)){
			$a += 0x10000000;
			
			if($main->{chb4}->Checked() or $main->{chb5}->Checked() or $main->{chb6}->Checked() or $main->{chb7}->Checked() or $main->{chb8}->Checked() or $main->{chb9}->Checked() or $main->{chb10}->Checked()){
				$a &= 0xfffffff0 if($main->{chb4}->Checked());
				$a &= 0xffffff0f if($main->{chb5}->Checked());
				$a &= 0xfffff0ff if($main->{chb6}->Checked());
				$a &= 0xffff0fff if($main->{chb7}->Checked());
				$a &= 0xfff0ffff if($main->{chb8}->Checked());
				$a &= 0xff0fffff if($main->{chb9}->Checked());
				$a &= 0xf0ffffff if($main->{chb10}->Checked());	
			}
			$head .= "mac_${total_send} = " . change_pack($a);
			$total_send++;
		}else{
			last;
		}
	}
	$head .= "mac_num = $total_send\n";
	print MAC_TABLE $head;
	close MAC_TABLE;
}



sub change_pack{
	my $change_mac = shift;

	my $a4 = sprintf("%02x", $change_mac&0xff);
	my $a5 = sprintf("%02x", ($change_mac>>8)&0xff);
	my $a6 = sprintf("%02x", ($change_mac>>16)&0xff);
	my $a7 = sprintf("%02x", ($change_mac>>24)&0xff);

	my $aa = $a7 . " " . $a6 . " " . $a5 . " " . $a4 . " 22 44 " . $a7 . " " . $a6 . " " . $a5 . " " . $a4 . " 33 44 \n"; 
	#my $my_packet = $local_mac . $aa . $opt_code . $tpacket;
	return $aa;
}