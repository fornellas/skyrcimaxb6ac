#!/usr/bin/perl

use strict;
use warnings;

my $input = *STDIN;

sub hexdump (@) {
	my $t = join('', map { sprintf("%02x", $_) } @_ );
	$t =~ s/(........)/$1 /g;
	return $t;
}

use Data::Dumper;

sub array_equal {
	my @a = @{$_[0]};
	my @b = @{$_[1]};
	return 0 unless scalar(@a) == scalar(@b);
	for( my $i = 0; $i < scalar(@a); $i++ ) {
		return 0 unless $a[$i] == $b[$i];
	}
	return 1;
}


my %mode = (0x00 => "Config", 0x05 => "Save", 0x06 => "Load",
            0x01 => "Li", 0x02 => "NiMH", 0x03 => "NiCd", 0x04 => "Pb" );
my %running = ( 0x00 => "Standby", 0x01 => "Running" );

my $buf = ''; my $new;
while( sysread($input, $new, 255) and $buf .= $new ) {
	while( $buf =~ s/\{(.{74})\}// ) {
		if( $-[0] != 0 ) { # We have garbage before the match
			print STDERR "Resynced\n";
			$buf = substr $buf, $-[0];
		}
		my @data = map {ord($_)} split //, $1;

		foreach( @data[0..71] ) { # data is 0b1_______ x 72
			if( ($_ & 0x80) != 0x80 ) {
				print STDERR "Assertion failed: in data: $_ & 0x80 != 0x80\n", hexdump(@data), "\n";
				next;
			}
		}

		foreach( @data[72..73] ) { # Checksum is 0x3_ 0x3_
			if( ($_ & 0xf0) != 0x30 ) {
				print STDERR "Assertion failed: in checksum: $_ & 0xf0 != 0x30\n", hexdump(@data), "\n";
				next;
			}
		}

		my $sum = 0; $sum += $_ for( @data[0..71] );
		$sum &= 0xff;
		my $their_sum = (($data[72] & 0x0f) << 4) | ($data[73] & 0x0f);
		if( $sum != $their_sum ) {
			print STDERR "Checksum error: received $their_sum, calculated $sum\n";
		}

		$_ &= 0x7f foreach(@data); # Unset high bit


		unless( array_equal([@data[0..6]], [0x1e, 0x04, 0x04, 0x50, 0x05, 0x00, 0x6e] ) ) { print STDERR "Unknown data in bytes 0-6: ", hexdump(@data[0..6]), "\n"; }
		unless( ($data[7] & 0xee) == 0x00 ) { print STDERR "Unknown state in byte 7: ", hexdump($data[7]), "\n"; }
		my $cycle = ($data[7] & 0x10) >> 4;
		my $direction = ( ($data[7] & 0x01) ? 1 : -1 );
		my $NiCd_charge_current = $data[8]/10;
		my $NiCd_discharge_current = $data[9]/10;
		unless( array_equal([@data[10..11]], [0x00, 0x01]) ) { print STDERR "Unknown data in bytes 10-11: ", hexdump(@data[10..11]), "\n"; }
		my $NiMH_charge_current = $data[12]/10;
		my $NiMH_discharge_current = $data[13]/10;
		my $cycle_mode = $data[14];
		my $cycle_count = $data[15];
		my $LiPo_charge_current = $data[16]/10;
		my $LiPo_cells = $data[17];
		my $LiPo_discharge_current = $data[18]/10;
		my $LiPo_discharge_cells = $data[19];
		my $Pb_charge_current = $data[20]/10;
		my $Pb_cells = $data[21];
		my $mode = $mode{ $data[22] } or print STDERR "Unknown mode $data[22]\n";
		my $running = $running{ $data[23] } or print STDERR "unknown state $data[24]\n";
		my $NiMH_discharge_voltage = $data[24]*10 + $data[25]/10;
		my $NiCd_discharge_voltage = $data[26]*10 + $data[27]/10;
		unless( array_equal([@data[28..31]], [0x00, 0x0c, 0x32, 0x00]) ) { print STDERR "Unknown data in bytes 28-31: ", hexdump(@data[28..31]), "\n"; }
		my $current = $data[32] + $data[33]/100;
		my $voltage = $data[34] + $data[35]/100;
		unless( array_equal([@data[36..39]], [0x00, 0x00, 0x00, 0x00]) ) { print STDERR "Unknown data in bytes 36-39: ", hexdump(@data[36..39]), "\n"; }
		my $in_voltage = $data[40] + $data[41]/100;
		my $charge  = $data[42]*100 + $data[43];
		my @cellvoltage;
		$cellvoltage[0] = $data[44] + $data[45]/100;
		$cellvoltage[1] = $data[46] + $data[47]/100;
		$cellvoltage[2] = $data[48] + $data[49]/100;
		$cellvoltage[3] = $data[50] + $data[51]/100;
		$cellvoltage[4] = $data[52] + $data[53]/100;
		$cellvoltage[5] = $data[54] + $data[55]/100;
		my @thirteenzero = (0x00) x 13;
		unless( array_equal([@data[56..68]], \@thirteenzero) ) { print STDERR "Unknown data in bytes 56-68: ", hexdump(@data[56..68]), "\n"; }
		my $time = $data[69];
		unless( array_equal([@data[70..71]], [0x0f, 0x24]) ) { print STDERR "Unknown data in bytes 70-71: ", hexdump(@data[70..71]), "\n"; }

		if( $running eq "Standby" ) {
			printf "Standby\n";

		} elsif( $running eq "Running" ) {
			my $state;
			if( $cycle ) {
				if( ($cycle_mode & 0xfe) != 0x00 ) { print STDERR "Unknown cycle mode $cycle_mode\n"; }
				$state = "cycle (" .
				         ( ($cycle_mode&0x01) ? "C>D" : "D>C" ) .
				         " x" . $cycle_count .
				         ") ";
			} else {
				$state = "single ";
			}
			$state .= $direction == 1 ? "charging" : "discharging";
			if( $mode eq "Li" ) {
				my $set_current = ($direction == 1 ? $LiPo_charge_current : $LiPo_discharge_current);
				printf "Uin=%4.2f V  %s Li  t=%d min  Iset=%4.2f A  Cells=%d  Iout=%4.2f A  Uout=%4.2f V  Qout=%d mAh  ",
				       $in_voltage, $state, $time, $set_current, $LiPo_cells, $current, $voltage, $charge;
				for(my $i=0; $i < $LiPo_cells; $i++) {
					printf "U%d=%4.2f V ", $i+1, $cellvoltage[$i];
				}
				print "\n";

			} elsif( $mode eq "NiMH" ) {
				my $set_current = ($direction == 1 ? $NiMH_charge_current : $NiMH_discharge_current);
				my $set_voltage = ($direction == -1 ? " Uset=".$NiMH_discharge_voltage." V " : "");
				printf "Uin=%4.2f V  %s NiMH  t=%d min  Iset=%4.2f A %s Iout=%4.2f A  Uout=%4.2f V  Qout=%d mAh  \n",
				       $in_voltage, $state, $time, $set_current, $set_voltage, $current, $voltage, $charge;

			} elsif( $mode eq "NiCd" ) {
				my $set_current = ($direction == 1 ? $NiCd_charge_current : $NiCd_discharge_current);
				my $set_voltage = ($direction == -1 ? " Uset=".$NiCd_discharge_voltage." V " : "");
				printf "Uin=%4.2f V  %s NiCd  t=%d min  Iset=%4.2f A %s Iout=%4.2f A  Uout=%4.2f V  Qout=%d mAh  \n",
				       $in_voltage, $state, $time, $set_current, $set_voltage, $current, $voltage, $charge;

			} elsif( $mode eq "Pb" ) {
				my $set_current = ($direction == 1 ? $Pb_charge_current : "?");
				printf "Uin=%4.2f V  %s Pb  t=%d min  Iset=%4.2f A  Cells=%d  Iout=%4.2f A  Uout=%4.2f V  Qout=%d mAh  \n",
				       $in_voltage, $state, $time, $set_current, $Pb_cells, $current, $voltage, $charge;

			} else {
				print STDERR "Unknown mode $mode\n";
			}

		} else {
			print STDERR "Unknown state $running\n";
		}

	}
}
