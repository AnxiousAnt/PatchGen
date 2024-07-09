#!/usr/bin/perl

use strict;
use warnings;

my $library = "";
my $name = "";

foreach (`/sw/bin/find /Applications/Pd-extended.app/Contents/Resources/doc/5.reference/ -type f -name '*.pd'`) {
	 chop;
	 if(m|.*/doc/5\.reference/([a-zA-Z0-9_-]+)/(.+)-help\.pd|) {
		  print("library: $1  name: $2\n");
		  $library = lc($1);
		  $name = $2;
	 } elsif (m|.*/doc/5\.reference/([a-zA-Z0-9_-]+)/(.+)\.pd|) {
		  print("library: $1  name: $2 \t\t(no -help)\n");
		  $library = lc($1);
		  $name = $2;
	 }
#	 else {
#		  print("FAILED PARSE: $_\n");
#	 }
}
