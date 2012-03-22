#!/usr/bin/perl -w
use warnings;
use strict;
use File::Basename;
use Time::HiRes qw(usleep  gettimeofday tv_interval stat );
use Getopt::Long;
use Encode;
use Win32::GUI;
use Win32 ();
use Win32::GUI::Loft;
use Net::SSH::W32Perl;
use threads;
use Data::Dumper;
my $wireshark_name='WinPcap_4_1_2.exe';
my $pscp_name = 'PSCP.EXE';
eval{
	require Net::Pcap;
};

my %tmp_devinfo;
my $tmp_err;
my @tmp_devs = Net::Pcap::findalldevs(\%tmp_devinfo, \$tmp_err);
if($@ or @tmp_devs==0){
	my $wireshark_path=dirname($0)."/$wireshark_name";
	unless(-e $wireshark_path){
		$wireshark_path = PerlApp::extract_bound_file($wireshark_name);
	}
	system($wireshark_path);
}

require Net::Pcap;
require Net::PcapUtils;
my $main;
my $dev;
my $rarp_all=1;
my $rarp_mac="FF:FF:FF:FF:FF:FF";

my $win_file=dirname($0)."/main.gld";
unless(-e $win_file){
	$win_file = PerlApp::extract_bound_file('main.gld');
}
my $objDesign = Win32::GUI::Loft::Design->newLoad($win_file) or die("Could not open window file ($win_file)");
$main = $objDesign->buildWindow( undef ) or die("Could not build window ($win_file)");

my %devinfo;
my $err;
my @host_mac=(0,1,2,3,4,5);
use   constant   {  
	RELAY_TOKEN_TYPE_RARP => 16,
	RELAY_TOKEN_TYPE_RARP_ACK => 17,
	RELAY_TOKEN_TYPE_FACTORY_SET => 18,
	RELAY_TOKEN_TYPE_FACTORY_SET_ACK => 19,
};

my @devs = Net::Pcap::findalldevs(\%devinfo, \$err);
print Dumper(\%devinfo);
foreach (@devs) {
	print "$_ : $devinfo{$_}\n";
}

$main->cbDev->Add(map{$devinfo{$_}} @devs);      #网卡选择处添加数据
$main->cbDev->SelectString($devinfo{$devs[1]});  #选择默认的网卡
$main->tfRarpMac->Text($rarp_mac);
$main->chbRarpAll->Checked($rarp_all);


$main->reEdit->Text("\r\n
	杭州蓝联科技有限公司\r\n
	帮助说明:\r\n
	1.本工具用来查找局端ip设置和恢复出厂设置\r\n
	2.网卡选择栏选择好连接局端的网卡\r\n
	3.MAC地址处填写好局端MAC地址\r\n
	4.勾上查找所有能查找出在局域网里所有局端\r\n
	5.点击查找按钮开始查找\r\n
	6.点击恢复出厂设置或恢复IP地址或删除VLAN\r\n
	"
);

#$SIG{__DIE__}=sub{ };

$main->Show();
#my ($DOS) = Win32::GUI::GetPerlWindow(); Win32::GUI::Hide($DOS);

Win32::GUI::Dialog();

#alarm(0.1);

sub __get_params{
	$dev=$devs[$main->cbDev->GetCurSel()];

	my $ret=`ipconfig/all`;
	my $filter=substr($devinfo{$dev},0,10);
	if($ret=~/\Q$filter\E.*?\n\s+Physical Address.*?([0-9a-fA-F]{2}-.{14})/s){
		#@host_mac=map{hex $_}split "-",$1;
	}
	$rarp_mac=$main->tfRarpMac->Text();
	$rarp_all=$main->chbRarpAll->Checked();
}

sub get_rarp_pack{
	my @dst_mac=map{hex $_}split ":",$rarp_mac;
	my @arry=(@dst_mac,@host_mac
		,0x89,0x89
		,0,0
		,0,24 #len
		,0,0 #cs
		,0
		,RELAY_TOKEN_TYPE_RARP	
		,0,0
	);
	my $size=@arry;
	print "arry size=$size";
	my $cs=msys_checksum16(pack("C*",@arry[16..$size-1]));
	if($cs != 0xffff){
		$cs = ~$cs;
		$cs &= 0xffff;
	}


	$arry[18]=$cs>>8;
	$arry[19]=$cs&0xff;
	
	while(@arry<60){
		push @arry,0;
	}
	return pack("C*",@arry);
}

sub pcap_thread{
	$SIG{'INT'} = sub { threads->exit(); };
	my $pcap=Net::Pcap::open_live($dev, 1516, 1, 10, \$err) or die "$! $err";
	my $net;
	my $mask;
	Net::Pcap::lookupnet($dev, \$net, \$mask, \$err); print "$net $mask\n";

	#my $filter_str="ether proto 0x8989 and not ether src " . (join ":",(map{sprintf("%02x",$_)} @host_mac));
	my $filter_str="ether proto 0x8989";
	print "$filter_str\n";
	my $program;
	Net::Pcap::compile($pcap, \$program, $filter_str, 10, $mask) and die "compile(): check string with filter";
	Net::Pcap::setfilter($pcap,$program);
	my $packet=get_rarp_pack();
	Net::Pcap::sendpacket($pcap, $packet);

	print "start loop\n";
	Net::Pcap::loop($pcap, 1000, \&process_packet, "just for the demo");#这里调用一个回调函数写ret.txt
	print "stop loop\n";

	#$text=~s{\nlo.+?\n}{\n}gm;
	#$text=~s{\n.+?127\.0\.0\.1.+?\n}{\n}gm;

	print "Net::Pcap::close\n";
	Net::Pcap::close($pcap);
}

sub rarp{
	__get_params();#获取必要的参数

	if($rarp_all){
		$rarp_mac="FF:FF:FF:FF:FF:FF";
	}

	$main->reEdit->Text("开始查找...\r\n");
	unlink("ret.txt");
	open RET,">ret.txt" or die $!;
	binmode RET;
	close RET;
	my $thr=threads->create(\&pcap_thread);
	sleep 3;
	$thr->kill('SIGINT');

	open RET,"<ret.txt" or die $!;
	binmode RET;
	local $/;


	my $text='';
	while(<RET>){
		$text .= "$_\r\n"; 
	}
	close RET;
	$main->reEdit->Text($text);
	return 0;
}
#这个是点击查找按钮后执行的程序
sub btnRarp_Click{
	rarp();
	0;
}

sub get_factory_pack{
	my $vlan=shift || 0;

	my @dst_mac=map{hex $_}split ":",$rarp_mac;
	my @arry=(@dst_mac,@host_mac
		,0x89,0x89
		,0,0
		,0,24+32+64#len
		,0,0 #cs
		,0
		,RELAY_TOKEN_TYPE_FACTORY_SET
		,0,0
		,0..31
		,0..63
	);

	my $size=@arry;
	print "arry size=$size";
	my $cs=msys_checksum16(pack("C*",@arry[16..$size-1]));
	if($cs != 0xffff){
		$cs = ~$cs;
		$cs &= 0xffff;
	}
	$arry[18]=$cs>>8;
	$arry[19]=$cs&0xff;
	

	if($vlan){
		@arry =( @arry[0..11] ,0x81,0x00,0x00,0x01,@arry[12..$size-1] );
	}
	while(@arry<60){
		push @arry,0;
	}
	return pack("C*",@arry);
}

sub factory{
	__get_params();
	$main->reEdit->Text("开始恢复出厂设置和重启...\r\n");

	my $pcap=Net::Pcap::open_live($dev, 1516, 0, 10, \$err) or die "$! $err";
	my $net;
	my $mask;
	Net::Pcap::lookupnet($dev, \$net, \$mask, \$err); print "$net $mask\n";
	my $packet;
       	$packet= get_factory_pack(0);
	Net::Pcap::sendpacket($pcap, $packet);
       	$packet= get_factory_pack(1);
	Net::Pcap::sendpacket($pcap, $packet);
	Net::Pcap::close($pcap);
	$main->reEdit->Text("OK");
}
sub btnPing{
#	printf("不支持\n\n");
#	return 0;

	printf("btnPing_Click\n\n");
	__get_params();
	my $pcap=Net::Pcap::open_live($dev, 1516, 0, 10, \$err) or die "$! $err";
	my $net;
	my $mask;
	Net::Pcap::lookupnet($dev, \$net, \$mask, \$err); print "$net $mask\n";
	my $packet;

		my $filter_str="ether proto 0x8989 and not ether src " . (join ":",(map{sprintf("%02x",$_)} @host_mac));
		print "$filter_str\n";
		my $program;
		Net::Pcap::compile($pcap, \$program, $filter_str, 10, $mask) and die "compile(): check string with filter";
		Net::Pcap::setfilter($pcap,$program);

	my @ps=qw/
52 54 00 00 99 55	 00  21   9b df 9f 8a 81 00 60 6c 08 06 00 01 
08 00 06 04 00 01 10 01   00 03 00 00 0a 07 01 1f 
10 01 00 03 00 00 0a 07   01 1f 
/;

	$packet=pack("C*",map{hex $_}@ps);

	Net::Pcap::sendpacket($pcap, $packet);
	0;
}


#这个是点击恢复出厂设置后程序
sub btnFactory_Click{
#btnPing();
factory();
	0;
}

sub process_packet {
	my($user_data, $header, $packet) = @_;

	if(substr($packet,12,2) eq pack("C*",0x81,0x00)){
		substr($packet,12,4)='';
	}

	#print "process_packet\n";
	if(substr($packet,12,2) eq pack("C*",0x89,0x89)){
		if(substr($packet,21,1) ne pack("C",RELAY_TOKEN_TYPE_RARP_ACK)){
			return;
		}

		my $ret=substr($packet,24);
		$ret=~s/\x00//gs;
		$ret=~s/\n/\r\n/g;
		open RET,">>:unix","ret.txt" or die $!;
		binmode RET;
		print RET "=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*\r\n";
		print RET "$ret\r\n";
		print $ret;
		close RET;
	}
	# do something ...
}

sub msys_checksum16{
	my $buf = shift @_;
	my $len = length($buf);
	my $n = $len / 2;
	my $ret = 0;

	for(my $i = 0; $i < $n; ++$i){
		my @vals = unpack("C2", substr($buf, $i*2, 2));
		$ret += ($vals[0]<<8) | $vals[1];
		$ret &= 0xffffffff;
	}
	if($len & 1){
		$ret += unpack("C", substr($buf, $len - 1, 1)) << 8;
		$ret &= 0xffffffff;
	}
	while($ret >> 16){
		$ret = ($ret & 0xffff) + ($ret >> 16);
	}
	return $ret;
}



#cai add 
BEGIN{
	push @INC,dirname($0);
	unlink "interfaces";
	unlink "ret.txt";
}

END{
	unlink "interfaces";
	unlink "ret.txt";
}
sub but_clear_ip_Click{
	
	print"but_clear_ip_click\n";
	my $ip = get_ip();
	if($ip ne 'err'){
		clear_ip($ip);
	}
	else{
		
		print"get ip err\n";
		
	}
	
}
sub btn_clear_vlan_Click{
	
	print"btn_clear_vlan_click\n";
	my $ip = get_ip();
	if($ip ne 'err'){

		clear_vlan($ip);

	}
	else{
		
		print"get ip err\n";
			
	}
	
}


sub clear_ip{

	my $ip = shift || '10.7.1.88';
	my $login_name = 'root';
	my $password = 'bluelink123';
	my $pscp_path=dirname($0)."\\$pscp_name";
	my $interface=dirname($0)."\\interfaces";
	creat_network_file($rarp_mac);
	my $cmd = "$pscp_path -q -scp -pw $password  $interface   $login_name\@$ip:/etc/network/interfaces"; 
	#print"cmd = $cmd\n";
	system($cmd);
	if($?){
		print "reconver ip failure $!\n";
	}
	else{

		print "reconver ip successful\n";
		$main->reEdit->Text("恢复IP:10.7.1.88 地址成功\r\n");

	}
}
sub clear_vlan{
	my $ip = shift || '10.7.1.88';
	my $login_name = 'root';
	my $password = 'bluelink123';
	my $cmd  = "cp -f /etc/ipgd/vlan_default.conf       /etc/ipgd/vlan.conf && ";
	   $cmd .= "cp -f /etc/ipgd/local8306_disable.conf  /etc/ipgd/local8306.conf";

	my $ipg_ssh = new Net::SSH::W32Perl($ip);
	$ipg_ssh->login($login_name,$password);
	my ($out, $err, $exit) = $ipg_ssh->cmd("$cmd");
	if($exit){

		print"recover vlan failure $err\n";
	}
	else{
	
		print"recover vlan successful\n";
		$main->reEdit->Text("恢复vlan成功\r\n");
	}

}

sub get_ip{
	open RET, "<ret.txt" or die "open ret.txt $!\n";
	my @ip_array = ();
	while(<RET>){
		#print"$_\n";
		if($_ =~ /inet addr:([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})\s+/){
			my $tmp = $1;
			if($tmp ne '127.0.0.1'){
				#print "============>>$1\n";
				push @ip_array,$tmp;
			}
		}
	}
	foreach(@ip_array){
		my $tmp_ip = $_;
		print"$_\n";
		my $ping_text = qx("ping $tmp_ip -n 2");
		if($ping_text =~ /Received\s+=\s+(\d+)/){
			
			#print"tmp_ip = $tmp_ip\n" if ($1);
			return $tmp_ip;
			
		}
	}
	return 'err';

}

sub creat_network_file{
	#这个函数建一个	interfaces文件

	my $ip = "10.7.1.88";
	my $netmask = "255.255.0.0";
	my $gateway = "10.7.0.1";
	my $mac = shift || "52:54:99:99:99:99";
	my $dns = "218.108.248.200 221.12.1.228";
	my $interfaces = "interfaces";
	unlink $interfaces;
	open  NETWORD_FILE,'>:unix',"$interfaces" or die "open creat_network_file error\n";
	my $creat_network_file = <<'NETWORD_FILE';
	######################################################################
# /etc/network/interfaces -- configuration file for ifup(8), ifdown(8)
#
# A "#" character in the very first column makes the rest of the line
# be ignored. Blank lines are ignored. Lines may be indented freely.
# A "\" character at the very end of the line indicates the next line
# should be treated as a continuation of the current one.
#
# The "pre-up", "up", "down" and "post-down" options are valid for all 
# interfaces, and may be specified multiple times. All other options
# may only be specified once.
#
# See the interfaces(5) manpage for information on what options are 
# available.
######################################################################

# We always want the loopback interface.
#
# auto lo
# iface lo inet loopback

# An example ethernet card setup: (broadcast and gateway are optional)
#
# auto eth0
# iface eth0 inet static
#     address 192.168.0.42
#     network 192.168.0.0
#     netmask 255.255.255.0
#     broadcast 192.168.0.255
#     gateway 192.168.0.1

# A more complicated ethernet setup, with a less common netmask, and a downright
# weird broadcast address: (the "up" lines are executed verbatim when the 
# interface is brought up, the "down" lines when it's brought down)
#
# auto eth0
# iface eth0 inet static
#     address 192.168.1.42
#     network 192.168.1.0
#     netmask 255.255.255.128
#     broadcast 192.168.1.0
#     up route add -net 192.168.1.128 netmask 255.255.255.128 gw 192.168.1.2
#     up route add default gw 192.168.1.200
#     down route del default gw 192.168.1.200
#     down route del -net 192.168.1.128 netmask 255.255.255.128 gw 192.168.1.2

# A more complicated ethernet setup with a single ethernet card with
# two interfaces.
# Note: This happens to work since ifconfig handles it that way, not because
# ifup/down handles the ':' any differently.
# Warning: There is a known bug if you do this, since the state will not
# be properly defined if you try to 'ifdown eth0' when both interfaces
# are up. The ifconfig program will not remove eth0 but it will be
# removed from the interfaces state so you will see it up until you execute:
# 'ifdown eth0:1 ; ifup eth0; ifdown eth0'
# BTW, this is "bug" #193679 (it's not really a bug, it's more of a 
# limitation)
#
# auto eth0 eth0:1
# iface eth0 inet static
#     address 192.168.0.100
#     network 192.168.0.0
#     netmask 255.255.255.0
#     broadcast 192.168.0.255
#     gateway 192.168.0.1
# iface eth0:1 inet static
#     address 192.168.0.200
#     network 192.168.0.0
#     netmask 255.255.255.0

# "pre-up" and "post-down" commands are also available. In addition, the
# exit status of these commands are checked, and if any fail, configuration
# (or deconfiguration) is aborted. So:
#
# auto eth0
# iface eth0 inet dhcp
#     pre-up [ -f /etc/network/local-network-ok ]
#
# will allow you to only have eth0 brought up when the file 
# /etc/network/local-network-ok exists.

# Two ethernet interfaces, one connected to a trusted LAN, the other to
# the untrusted Internet. If their MAC addresses get swapped (because an
# updated kernel uses a different order when probing for network cards,
# say), then they don't get brought up at all.
#
# auto eth0 eth1
# iface eth0 inet static
#     address 192.168.42.1
#     netmask 255.255.255.0
#     pre-up /path/to/check-mac-address.sh eth0 11:22:33:44:55:66
#     pre-up /usr/local/sbin/enable-masq
# iface eth1 inet dhcp
#     pre-up /path/to/check-mac-address.sh eth1 AA:BB:CC:DD:EE:FF
#     pre-up /usr/local/sbin/firewall

# Two ethernet interfaces, one connected to a trusted LAN, the other to
# the untrusted Internet, identified by MAC address rather than interface
# name:
#
# auto eth0 eth1
# mapping eth0 eth1
#     script /path/to/get-mac-address.sh
#     map 11:22:33:44:55:66 lan
#     map AA:BB:CC:DD:EE:FF internet
# iface lan inet static
#     address 192.168.42.1
#     netmask 255.255.255.0
#     pre-up /usr/local/sbin/enable-masq $IFACE
# iface internet inet dhcp
#     pre-up /usr/local/sbin/firewall $IFACE

# A PCMCIA interface for a laptop that is used in different locations:
# (note the lack of an "auto" line for any of these)
#
# mapping eth0
#    script /path/to/pcmcia-compat.sh
#    map home,*,*,*                  home
#    map work,*,*,00:11:22:33:44:55  work-wireless
#    map work,*,*,01:12:23:34:45:50  work-static
#
# iface home inet dhcp
# iface work-wireless bootp
# iface work-static static
#     address 10.15.43.23
#     netmask 255.255.255.0
#     gateway 10.15.43.1
#
# Note, this won't work unless you specifically change the file
# /etc/pcmcia/network to look more like:
#
#     if [ -r ./shared ] ; then . ./shared ; else . /etc/pcmcia/shared ; fi
#     get_info $DEVICE
#     case "$ACTION" in
#         'start')
#             /sbin/ifup $DEVICE
#             ;;
#         'stop')
#             /sbin/ifdown $DEVICE
#             ;;
#     esac
#     exit 0

# An alternate way of doing the same thing: (in this case identifying
# where the laptop is is done by configuring the interface as various
# options, and seeing if a computer that is known to be on each particular
# network will respond to pings. The various numbers here need to be chosen
# with a great deal of care.)
#
# mapping eth0
#    script /path/to/ping-places.sh
#    map 192.168.42.254/24 192.168.42.1 home
#    map 10.15.43.254/24 10.15.43.1 work-wireless
#    map 10.15.43.23/24 10.15.43.1 work-static
#
# iface home inet dhcp
# iface work-wireless bootp
# iface work-static static
#     address 10.15.43.23
#     netmask 255.255.255.0
#     gateway 10.15.43.1
#
# Note that the ping-places script requires the iproute package installed,
# and the same changes to /etc/pcmcia/network are required for this as for
# the previous example.


# Set up an interface to read all the traffic on the network. This 
# configuration can be useful to setup Network Intrusion Detection
# sensors in 'stealth'-type configuration. This prevents the NIDS
# system to be a direct target in a hostile network since they have
# no IP address on the network. Notice, however, that there have been
# known bugs over time in sensors part of NIDS (for example see 
# DSA-297 related to Snort) and remote buffer overflows might even be
# triggered by network packet processing.
# 
# auto eth0
# iface eth0 inet manual
# 	up ifconfig $IFACE 0.0.0.0 up
#       up ip link set $IFACE promisc on
#       down ip link set $IFACE promisc off
#       down ifconfig $IFACE down

# Set up an interface which will not be allocated an IP address by
# ifupdown but will be configured through external programs. This
# can be useful to setup interfaces configured through other programs,
# like, for example, PPPOE scripts.
#
# auto eth0
# iface eth0 inet manual
#       up ifconfig $IFACE 0.0.0.0 up
#       up /usr/local/bin/myconfigscript
#       down ifconfig $IFACE down

NETWORD_FILE
	print NETWORD_FILE $creat_network_file;
	print NETWORD_FILE "auto lo\n";
	print NETWORD_FILE "iface lo inet loopback\n\n";
	print NETWORD_FILE "auto eth0\n";
	print NETWORD_FILE "iface eth0 inet static\n";
	print NETWORD_FILE "	address $ip\n";
	print NETWORD_FILE "	netmask $netmask\n";
	print NETWORD_FILE "	gateway $gateway\n";
	print NETWORD_FILE "	hwaddress ether $mac\n";
	print NETWORD_FILE "	dns-nemserver $dns\n";
	close NETWORD_FILE;
}
