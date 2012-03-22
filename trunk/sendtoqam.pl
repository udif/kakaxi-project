#!/usr/bin/perl
eval 'exec perl -w -S $0 ${1+"$@"}'
    if 0;

use warnings;
use strict;
use IO::Socket::INET;
use File::Basename;

use IO::Handle;
use IO::Select;
use GET_IMG;
use Time::HiRes qw(usleep);
use threads;
use threads::shared; 
use Thread::Semaphore; 

my $g_url = "rtsp://10.7.1.251:3307/opt/movieTS/zhuzhuxia3_1.ts";
my $g_regionid="0x603";
my $g_vod_fd = 1;
my $img_clear_expire:shared = undef;
my $sem_thread:shared;
my $sem_img_clear:shared;
my $area1_img_state:shared;
my $get_img;
my $vod_sock;
my $vod_time_start;
my @clearlist:shared;


start_rtsp(@ARGV);

sub add_clear_event{
	my $name = shift;
	
	$sem_img_clear->down();
	push @clearlist,$name;
	$sem_img_clear->up();
}
sub del_clear_event{
	my $name = shift;

	$sem_img_clear->down();
	foreach (@clearlist){
		if ($_ eq $name){
			undef $_;
			$_='';
		}
	}
	$sem_img_clear->up();
}


sub send_img{
	my $name = shift;
	my $arg1 =shift;
	my $arg2 =shift;

	my ($x, $y, $img) = $get_img->get_img($name,$arg1,$arg2);
	if(defined $img){
		my $len = length $img;
		my $info = "x:$x y:$y len:$len\n";
		my $info_len = length $info;
		$info .= ("\0"x(64 - $info_len));
		my $ret = send_cmd($vod_sock,$img,13,$info);
	}
	$img_clear_expire = time() + 3;
}
sub send_clear_img{
	my $name = shift;

	my ($x, $y, $img) = $get_img->get_img("clear",$name );

	if(defined $img){
		my $len = length $img;
		my $info = "x:$x y:$y len:$len\n";
		my $info_len = length $info;
		$info .= ("\0"x(64 - $info_len));
		my $ret = send_cmd($vod_sock,$img,13,$info);
	}
}

sub clear_img{
	my $name = shift;
	
	if($name eq "area1" or $name eq "all"){
		del_clear_event("play");
		send_clear_img("play");
		del_clear_event("stop");
		send_clear_img("stop");
		del_clear_event("left");
		send_clear_img("left");
		del_clear_event("right");
		send_clear_img("right");
		undef $area1_img_state;
		return ;
	}
	if($name eq "area2" or $name eq "all"){
		del_clear_event("bar");
		send_clear_img("bar");
		return ;

	}
	del_clear_event($name);
	send_clear_img($name);
	if($name eq $area1_img_state){
		undef $area1_img_state;
	}
}

sub thread_clear_img{
	my $name;
	
	while(1){
		my $curr_time = time();
		if( defined $img_clear_expire and $img_clear_expire < $curr_time ){
			print "thread_clear_img : $curr_time\n";
			##show_info("cur time $curr_time expire time $img_clear_expire \n");
			$sem_img_clear->down();
			foreach $name(@clearlist){
				if(defined $area1_img_state and $name eq $area1_img_state){
					$area1_img_state = "null";
				}
				if($name eq "left" or $name eq "right" or $name eq "play" or $name eq "stop" ){
					$area1_img_state = "null";
				}
				##show_info("name is $name\n \n");
				send_clear_img($name);
			}
			undef @clearlist;
			undef $img_clear_expire;
			$sem_img_clear->up();
		}
		sleep(1);
	}
}

sub get_vod_cur_time{
	return time() - $vod_time_start;
}
sub show_info{
	my $str = shift;
	return ;
	open FILE,">>/tmp/forg/show";
	printf FILE "$str\n";
	close FILE;
}

sub build_pack{
        my $type = shift;
        my $data_in = shift;

        my $len = length $data_in;

        my $pack_len = pack("I",$len);
        my $pack_NULL_4 = pack("S",0);
        my $pack_type = pack("S",$type);
        my $pack = "$pack_len"."$pack_type"."$pack_NULL_4"."$data_in";
        return $pack;
}

sub send_cmd{
	my $socket = shift;
	my $buf = shift;
	my $type = shift;
	my $info = shift;
	my $pack;

	if(!defined $info){
		$info = "";
	}

	$pack = build_pack($type,"$info"."$buf");

	my $need_send = length $pack;
	my $off_set = 0;
	print "region pid : ================$ENV{X_REGION_PID} \n";
	if( $type == 13 and ( $ENV{X_REGION_PID} >=300 and $ENV{X_REGION_PID}<= 400)){
	}
	else{
		while($need_send > 0){
			my $ret = syswrite($socket,$pack, $need_send, $off_set) ;
			if($ret <= 0){
				die show_info("send_cmd_recv_ack err : syswrite err \n\t socket : $socket buf : $pack\n")
			}
			$need_send -= $ret;
			$off_set += $ret;
		}
	}
	
	return 1;
=pop	
	my $len = length $buf;
	my $head = "$type,$len,";
	$len = length $head;
	$head .= ("\0"x(32 - $len));
	$head .= "$info";
	$len = length $head;
	$head .= ("\0"x(128 - $len));

	if(!defined $sem_thread){
		$sem_thread = Thread::Semaphore->new(1); 
	}
	$sem_thread->down();
	syswrite($socket,$head) or die show_info("send_cmd_recv_ack err : syswrite err \n\t socket : $socket buf : $buf\n");
	syswrite($socket,$buf) or die show_info("send_cmd_recv_ack err : syswrite err \n\t socket : $socket buf : $buf\n");
	$sem_thread->up();
	return 1;
=cut
}

atexit{
	clear_img("area1");
	clear_img("area2");
};

sub start_rtsp{
	my $url=shift || $g_url;
	my $regionid=shift || $g_regionid;
	my $ser_url=shift || $g_vod_fd;
	my $vod_pause = 0;
	my $range_min=0;
	my $range_max=100;
	my $cur_time = 0;
	my $next_time = $cur_time ;
	my $audio_value = 128 ;
	my $audio_next = $audio_value ;
	my $wait_ok = 0;
	
	my $io = new IO::Handle;
	my $se = new IO::Select;

	my $server_sock=IO::Socket::UNIX->new(
                Peer=>$ser_url,
                Type=>SOCK_STREAM)
	or die "Concurrent Server ERROR:$!\n";

	##$vod_sock = $io->fdopen($server_sock,"w+");
	$vod_sock = $server_sock;
	$se->add($vod_sock);

	##show_info("$url $regionid $vod_fd\n");

	my $agent = 'HMTL RTSP 1.0';
	my $npt = '0-';

	my $describe='DESCRIBE $url RTSP/1.0\r\n'.
		'CSeq: $cseq\r\n'.
		'x-RegionID: $regionid\r\n'.
		'User-Agent: $agent\r\n\r\n';
	my $setup1 = 'SETUP $url/trackID=0 RTSP/1.0\r\n'.
		'CSeq: $cseq\r\n'.
		'Transport: MP2T\r\n'.
		'User-Agent: $agent\r\n\r\n';
	my $setup2 ='SETUP $url/trackID=1 RTSP/1.0\r\n'.
		'CSeq: $cseq\r\n'.
		'Session:$session\r\n'.
		'Transport: MP2T\r\n'.
		'User-Agent: $agent\r\n\r\n';
	my $play = 'PLAY $url RTSP/1.0\r\n'.
		'CSeq: $cseq\r\n'.
		'Range: npt=$npt\r\n'.
		'Scale: 1.0\r\n'.
		'Session:$session\r\n'.
		'x-prebuffer: maxtime=10.00\r\n'.
		'User-Agent: $agent\r\n\r\n';

	my $options='OPTIONS $url RTSP/1.0\r\n'.
		'CSeq: $cseq\r\n'.
		'User-Agent: $agent\r\n\r\n';
	my $pause= 'PAUSE $url RTSP/1.0\r\n'.
		'CSeq: $cseq\r\n'.
		'Session:$session\r\n'.  
		'User-Agent: $agent\r\n\r\n';
	my $teardown= 'TEARDOWN $url RTSP/1.0\r\n'.
		'CSeq: $cseq\r\n'.
		'Session:$session\r\n'.
		'User-Agent: $agent\r\n\r\n'; 
	my $play_after_pause = $play;
	$play_after_pause =~s/Range:.+?\\r\\n//;

#main
	#$SIG{__DIE__}=sub{print $_[0];print "enter key to exit\n";<>;};
	
	my $session;
	my $cseq=0;
	my $send_data;
	my $read_data;
	my $sock;
	my $ts_freq;
	my $pmt_pid;
	my $expire;
	my $send_and_read=sub{
		my $str=shift;
		$cseq++;
		$send_data=eval "\"$str\"";
		##show_info("[SEND]\n$send_data");
		syswrite($sock,$send_data) or die $!;
		sysread($sock,$read_data,4000) or die $!;
		##show_info("[RECV]\n$read_data\n");
	};

restart:
	my ($peer_addr,$peer_port)=$url=~/(\d+\.\d+\.\d+\.\d+):(\d+)/ or die "$url error";
	$sock = IO::Socket::INET->new(PeerAddr => $peer_addr, PeerPort => $peer_port, Proto => 'tcp') or exit 1;
	&$send_and_read($describe);
	while($read_data=~/Location:\s+(.+?)\s*$/g){
		$url=$1;
		goto restart;
	}
	
	($ts_freq,$pmt_pid)=$read_data=~/a=x-frequency:(\d+).+?a=x-pid:(\d+)/gs or die "describe error";
	##show_info( "ts_freq=$ts_freq,pmt_pid=$pmt_pid\n");
	##$io->syswrite("[global]\nts_freq=$ts_freq\npmt_pid=$pmt_pid\n");
	send_cmd($vod_sock,"[global]\nts_freq=$ts_freq\npmt_pid=$pmt_pid\n",14);
	&$send_and_read($setup1);
	($session)=$read_data=~/Session:(\s*\d+)/gs or die "not found session";
	&$send_and_read($setup2);
	&$send_and_read($play);
	($range_min,$range_max)=$read_data=~/Range: npt=(\d+)\.\d+-(\d+)?/gs or die "not found Range";
	$vod_time_start = time();
	##show_info( "range:$range_min - $range_max\n");
	$expire = time() + ($range_max - $range_min) + 3;
	$get_img = new GET_IMG(
		range_max =>$range_max,
		cur_time => $cur_time
		);

	my $option_expire = time() + 5;

	if(!defined $sem_img_clear){
		$sem_img_clear = Thread::Semaphore->new(1); 
	}

	my $thread_pid = threads->new ( \&thread_clear_img) or die $!;;
	while(1){
		my $curr_time = time();
		if($curr_time >= $expire ){
			&$send_and_read($teardown);
			show_info( "teardown\n");
			exit(0);
		}

		if($curr_time>=5){
			$option_expire = $curr_time + 5;
			&$send_and_read($options);
			exit(0) if($read_data=~/x-Info: \"CLOSE\"/gs);
		}

		if(!$se->can_read(5)){
			next;
		}
		my $line = <$vod_sock>;
		if(!defined $line){
			die "connect fail";
		}
		if($line=~/^key:(\w+)/){
			my $key = $1;
			if($key eq 'BC_PANEL_STOP' or $key eq 'BC_PANEL_EXIT'){
				clear_img("area1");
				clear_img("area2");
				my $ret = send_cmd($vod_sock,"",15);
				$line = <$vod_sock>;
				if($line=~/^key:(\w+)/){
					$key = $1;
					if($key eq 'BC_FORCE_EXIT_REQ_ACK'){
						&$send_and_read($teardown);
						exit(0);
					}
				}
			}

			if($key eq 'BC_PANEL_KILL'){
				&$send_and_read($teardown);
				exit(0);
			}

#ÔÝÍ£
			my $action = 'null';
			$action = 'play' if($vod_pause and $key eq 'BC_PANEL_PLAY');
			$action = 'play' if($vod_pause and $key eq 'BC_PANEL_SELECT');
			$action = 'pause' if(!$vod_pause and $key eq 'BC_PANEL_PAUSE');
			$action = 'pause' if(!$vod_pause and $key eq 'BC_PANEL_SELECT');
			$action = 'left' if($key eq 'BC_PANEL_LEFT' or $key eq 'BC_PANEL_PAGE_UP');
			$action = 'right' if($key eq 'BC_PANEL_RIGHT' or $key eq 'BC_PANEL_PAGE_DOWN');
			$action = 'up' if($key eq 'BC_PANEL_UP');
			$action = 'down' if($key eq 'BC_PANEL_DOWN');
			$action = 'ok' if( $wait_ok and $key eq 'BC_PANEL_SELECT');
			$action = 'audio' if($key eq 'BCC_MSG_VOLUME');
			$action = 'back' if($key eq 'BC_PANEL_BACK');
			

			#$ntp = "100-";
			#&$send_and_read($play);

			my $x = undef;
			my $y = undef;
			my $img = undef;
=pop
			if($wait_ok == 0){
				$cur_time = time() - $vod_time_start ;
			}
			show_info("get next time $next_time cur time $cur_time vod time start $vod_time_start range_max $range_max \n");
=cut
			if($action eq 'play'){
				$action = 'null';
				&$send_and_read($play_after_pause);
				$vod_pause = 0;
				clear_img("area1");
				send_img("play");
				add_clear_event("play");
			}
			if($action eq 'pause'){
				$action = 'null';
				$vod_pause = 1;
				&$send_and_read($pause);
				clear_img("area1");
				send_img("stop");
			}
			if($action eq 'left'){
				$action = 'null';
				if($wait_ok == 1){
					$next_time  -= 15;
				}else{
					$cur_time = time() - $vod_time_start ;
					$next_time  = $cur_time - 15;
				}
				if($next_time < 0){
					$next_time = 0;
					$vod_time_start = time();
					$cur_time = 0;
				}
				if(!defined $area1_img_state or $area1_img_state ne "left"){
					#clear_img("all");
					del_clear_event("left");
					send_img("left");
					$area1_img_state = "left";
				}
				del_clear_event("bar");
				send_img("bar",$cur_time, $next_time);

				$vod_time_start+=15;
				$wait_ok = 1;
			}
			if($action eq 'right'){
				$action = 'null';
				if($wait_ok == 1){
					$next_time  += 15;
				}else{
					$cur_time = time() - $vod_time_start ;
					$next_time  = $cur_time + 15;
				}
				show_info("next time $next_time range_max $range_max \n");
				if($next_time > $range_max){
					$next_time = $range_max;
					$vod_time_start = time() - $range_max;
					$cur_time = $range_max;
				}
				if(!defined $area1_img_state or $area1_img_state ne "right"){
					#clear_img("all");
					send_img("right");
					$area1_img_state = "right";
				}
				send_img("bar",$cur_time, $next_time);

				$vod_time_start-=15;
				$wait_ok = 1;
			}
			if($action eq 'audio'){
				$action = 'null';
				my $muteIcon = undef;
				my $volume = undef;
				if($line=~/^key:(\w+) (\d+) (\d+)$/){
					$muteIcon = $2;
					$volume = $3;
				}
				print "get $muteIcon $volume -----------------\n";
				if(!defined $area1_img_state or $area1_img_state ne "audio"){
					clear_img("all");
				}
				if(defined $muteIcon  and $muteIcon == 1){
					send_img("MuteIcon");
					$area1_img_state = "MuteIcon";
				}else{
					if(defined $area1_img_state and $area1_img_state eq "MuteIcon"){
						clear_img("MuteIcon");
					}
					send_img("audio",$volume, $volume);
					add_clear_event("bar");
				}
			}
			
			if($action eq 'ok'){
				$action = 'null';
				$cur_time = $next_time;
				$npt = "$next_time-";
				&$send_and_read($play);
				send_img("bar",$cur_time, $next_time);
				add_clear_event("bar");
				add_clear_event("left");
				##add_clear_event("bar");
				$wait_ok = 0;
			}
			
			if($action eq 'back'){
				$action = 'null';
				if($wait_ok){
					clear_img("area1");
					clear_img("area2");
					$wait_ok = 0;
				}
			}
		}
	}
}

__END__

DESCRIBE rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts RTSP/1.0

CSeq: 1

x-RegionID: 0x603

User-Agent: HMTL RTSP 1.0



RTSP/1.0 200 OK

Server: UServer 3.1.18

Cseq: 1

Cache-Control: must-revalidate

Last-Modified: Mon, 18 Jul 2011 02:42:48 GMT

Date: Mon, 18 Jul 2011 02:42:48 GMT

Expires: Mon, 18 Jul 2011 02:42:48 GMT

Content-Base: rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts/

Content-Length: 345

Content-Type: application/sdp



v=0

o=OnewaveUServerEagle 719885386 424238335 IN IP4 10.7.100.9

s=/opt/movieTS/zhuzhuxia3_1.ts

u=http:///

e=admin@

c=IN IP4 0.0.0.0

t=0 0

a=isma-compliance:1,1.0,1

a=Range:npt=0.000000-665.840000

a=x-frequency:658000

a=x-pid:68

a=x-bitrate: 3617246

m=video 0 MP2T mpgv

a=control:trackID=0

m=audio 0 MP2T mpga

a=control:trackID=1

SETUP rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts/trackID=0 RTSP/1.0

CSeq: 2

Transport: MP2T

User-Agent: HMTL RTSP 1.0



RTSP/1.0 200 OK

Server: UServer 3.1.18

Cseq: 2

Session: 7085667359969386281

Cache-Control: no-cache

Transport: MP2T



SETUP rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts/trackID=1 RTSP/1.0

CSeq: 3

Session: 7085667359969386281

Transport: MP2T

User-Agent: HMTL RTSP 1.0



RTSP/1.0 200 OK

Server: UServer 3.1.18

Cseq: 3

Session: 7085667359969386281

Cache-Control: no-cache

Transport: MP2T



PLAY rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts RTSP/1.0

CSeq: 4

Range: npt=0-

Scale: 1.0

Session: 7085667359969386281

x-prebuffer: maxtime=10.00

User-Agent: HMTL RTSP 1.0



RTSP/1.0 200 OK

Server: UServer 3.1.18

Cseq: 4

Session: 7085667359969386281

Range: npt=0.000000-665.840000

x-Ts-info: 373000

Scale: 1.0



PAUSE rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts RTSP/1.0

CSeq: 6

Session: 7085667359969386281

User-Agent: HMTL RTSP 1.0



RTSP/1.0 200 OK

Server: UServer 3.1.18

Cseq: 6

Session: 7085667359969386281



OPTIONS rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts RTSP/1.0

CSeq: 7

Session: 7085667359969386281

User-Agent: HMTL RTSP 1.0



RTSP/1.0 200 OK

Server: UServer 3.1.18

Cseq: 7

Session: 7085667359969386281

Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, SCALE, GET_PARAMETER



PLAY rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts RTSP/1.0

CSeq: 9

Scale: 1.0

Session: 7085667359969386281

x-prebuffer: maxtime=10.00

User-Agent: HMTL RTSP 1.0



RTSP/1.0 200 OK

Server: UServer 3.1.18

Cseq: 9

Session: 7085667359969386281

Range: npt=4.880000-665.840000

x-Ts-info: 5253000

Scale: 1.0



OPTIONS rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts RTSP/1.0

CSeq: 10

Session: 7085667359969386281

User-Agent: HMTL RTSP 1.0



RTSP/1.0 200 OK

Server: UServer 3.1.18

Cseq: 10

Session: 7085667359969386281

Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, SCALE, GET_PARAMETER



PLAY rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts RTSP/1.0

CSeq: 12

Range: npt=73-

Scale: 1.0

Session: 7085667359969386281

x-prebuffer: maxtime=10.00

User-Agent: HMTL RTSP 1.0



RTSP/1.0 200 OK

Server: UServer 3.1.18

Cseq: 12

Session: 7085667359969386281

Range: npt=73.040000-665.840000

x-Ts-info: 73413000

Scale: 1.0



OPTIONS rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts RTSP/1.0

CSeq: 13

Session: 7085667359969386281

User-Agent: HMTL RTSP 1.0



RTSP/1.0 200 OK

Server: UServer 3.1.18

Cseq: 13

Session: 7085667359969386281

Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, SCALE, GET_PARAMETER



PLAY rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts RTSP/1.0

CSeq: 15

Range: npt=49-

Scale: 1.0

Session: 7085667359969386281

x-prebuffer: maxtime=10.00

User-Agent: HMTL RTSP 1.0



RTSP/1.0 200 OK

Server: UServer 3.1.18

Cseq: 15

Session: 7085667359969386281

Range: npt=49.040000-665.840000

x-Ts-info: 49413000

Scale: 1.0



TEARDOWN rtsp://10.7.100.9:3306/opt/movieTS/zhuzhuxia3_1.ts RTSP/1.0

CSeq: 16

Session: 7085667359969386281

User-Agent: HMTL RTSP 1.0



RTSP/1.0 200 OK

Server: UServer 3.1.18

Cseq: 16

Session: 7085667359969386281

Connection: Close




