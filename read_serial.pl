#!/usr/bin/perl

use strict;
use warnings;

use POSIX;
use Fcntl;

die("Usage: $0 </dev/ttyS1>\n") unless $#ARGV == 0;

my $TTS = $ARGV[0];
my $input;

# somehow sysopen hangs if it's not opened in NONBLOCK mode
sysopen($input, $TTS, O_RDONLY | O_NOCTTY | O_NONBLOCK)
	|| die("Couldn't open \"$TTS\": $!");

{ # Set back to blocking mode
	my $flags = fcntl($input, F_GETFL, 0)
		|| die("Couldn't get current flags");
	fcntl($input, F_SETFL, $flags & ~O_NONBLOCK)
		|| die("Couldn't set current flags");
}

select((select(*STDOUT), $| = 1)[0]);    # autoflush on
while( sysread $input, my $new, 255 ) {
	print $new;
}
