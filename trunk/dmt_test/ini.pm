
package ini;

use warnings;
use strict;

use Exporter;
our @ISA = qw/Exporter/;
our @EXPORT=qw/
ini_parser_file
ini_parser_str
ini_save_file
ini_update_file
ini_delete_file
/;

#ini_parser_file("myini.ini");
sub ini_parser_file{
	open FILE,"<$_[0]" or die $!;
	flock(FILE,2);
	binmode FILE;
	my @data=<FILE>;
	flock(FILE,8);
	close FILE;
	ini_parser_arry(\@data);
}

#ini_parser_str("aa=b \n cc=d\n");
sub ini_parser_str{
	my $ini={};
	my @lines=split /\n/,$_[0];
	ini_parser_arry(\@lines);
}

#ini_parser_str(["aa=b","cc=d"]);
sub ini_parser_arry{
	my $datap=shift;
	my $ini={};
	my $section="global";
	foreach(@{$datap}){
		$section=$1,next if(/^\[\s*(\S+)\s*\]/o);
		if(/^\s*([^\s;]+)\s*=\s*([^\r\n\t;]*)/o){
			my $key=$1;
			my $val=$2;
			$val=~s/\s+$//;
			$ini->{$section}{$key}=$val;
		}
	}
	$ini;
}

sub ini_delete_section{
	my $datap=shift;
	my $section=shift;
	my $find_section=0;
	my $line_num=0;
	foreach(@{$datap}){
		if(/^\[\s*(\S+)\s*\]/o){
			if($section eq $1){
				$find_section=1;
				$datap->[$line_num]="";
			}
			else{
				if($find_section){
					return;
				}
			}
		}
		elsif($find_section){
			$datap->[$line_num]="";
		}
		$line_num++;
	}
}

sub ini_delete_key_arry{
	my $datap=shift;
	my $section=shift;
	my $key=shift;
	my $line_num=0;
	my $find_section=0;
	foreach(@{$datap}){
		if(/^\[\s*\Q$section\E\s*\]/o){
			$find_section=1;
		}
		elsif($find_section){
			if($_=~m{^(\s*\Q$key\E\s*=\s*)}){
				$datap->[$line_num]="";
				return;
			}
		}
		$line_num++;
	}
}

sub ini_insert_key_arry{
	my $datap=shift;
	my $section=shift;
	my $key=shift;
	my $value=shift;

	my $line_num=0;

	foreach(@{$datap}){
		if(/^\[\s*\Q$section\E\s*\]/){
			my $line_max=@{$datap} - 1;
			@{$datap}=(@{$datap}[0..$line_num],"$key=$value\n",@{$datap}[$line_num+1..$line_max]);
			return;
		}
		$line_num++;
	}
	push @{$datap},"$key=$value\n";
}

#ini_parser_str(["aa=b","cc=d"],"global","web_pid_start","100");


sub ini_replace_value_arry{
	my $datap=shift;
	my $section=shift;
	my $key=shift;
	my $value=shift;
	my $old_value=shift;

	my $find_section=0;
	my $find_key=0;
	foreach(@{$datap}){
		if(/^\[\s*(\S+)\s*\]/o){
			if($section eq $1){
				$find_section=1;
			}
		}
		elsif($find_section){
			if($_=~s{^(\s*\Q$key\E\s*=[ ]*)[^\r\n\t;]*}{$1$value}){
				return;
			}
		}
	}
	push @{$datap},"$key=$value\n";
}

#ini_save_file("myini.ini",{"global"=>{"web_pid_start"=>"100"}};
sub ini_save_file{
	my $file=shift;
	my $up_ini=shift;
	open FILE,">$file" or die $!;
	binmode FILE;
	foreach my $section(keys %{$up_ini}){
		print FILE "\n[$section]\n";
		foreach my $key(keys %{$up_ini->{$section}}){
			print FILE "$key=$up_ini->{$section}{$key}\n";
		}
	}
	close FILE;
}

#use Data::Dumper;
sub ini_delete_file{
	my $file=shift;
	my $up_ini=shift;

	open FILE,"<$file" or die $!;
	flock(FILE,2);
	binmode FILE;
	my @data=<FILE>;
	flock(FILE,8);
	close FILE;

	my $file_ini=ini_parser_arry(\@data);
#print Dumper($file_ini);
	foreach my $section(keys %{$up_ini}){
		foreach my $key(keys %{$up_ini->{$section}}){
			if($key eq '__delete_all_key__'){
				ini_delete_section(\@data,$section);		
				delete $file_ini->{$section};
				last;
			}

			if(!exists $file_ini->{$section}{$key}){
				next;
			}
			ini_delete_key_arry(\@data,$section,$key);		
			delete $file_ini->{$section}{$key};
		}
	}

	open TMP,"<$file" or die $!;
	flock(TMP,2);

	open FILE,">$file" or die $!;
	binmode FILE;
	print FILE @data;
	close FILE;

	flock(TMP,8);
	close TMP;
}

#ini_update_file("myini.ini",{"global"=>{"web_pid_start"=>"100"}};
sub ini_update_file{
	my $file=shift;
	my $up_ini=shift;
	my @data=();

	if(open FILE,"<$file"){
		flock(FILE,2);
		binmode FILE;
		@data=<FILE>;
		flock(FILE,8);
		close FILE;
	}
	else{
		open TMP,">$file" or die $!;
		close TMP;
	}
	
	my $file_ini=ini_parser_arry(\@data);
	
	foreach my $section(keys %{$up_ini}){
		foreach my $key(keys %{$up_ini->{$section}}){
			my $value=$up_ini->{$section}{$key};
			if(!exists $file_ini->{$section}){
				push @data,"[$section]\n";
				push @data,"$key=$value\n";
				$file_ini->{$section}{$key}=$value;
			}
			elsif(!exists $file_ini->{$section}{$key}){
				ini_insert_key_arry(\@data,$section,$key,$value);		
				$file_ini->{$section}{$key}=$value;
			}
			elsif($value ne $file_ini->{$section}{$key}){
				ini_replace_value_arry(\@data,$section,$key,$value,$file_ini->{$section}{$key});		
				$file_ini->{$section}{$key}=$value;
			}
		}
	}

	#open TMP,"<$file" or die $!;
	#flock(TMP,2);
	open FILE,">$file" or die $!;
	binmode FILE;
	print FILE @data;
	close FILE;
	#flock(TMP,8);
	#close TMP;
}

1;

