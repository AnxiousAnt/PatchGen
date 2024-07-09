#!/usr/bin/perl 
use strict;
use warnings;

use WWW::Mechanize;
use Net::OpenSoundControl::Client;
use Data::Dumper;
use Time::HiRes qw(usleep);

my $client = Net::OpenSoundControl::Client->new(Port => 1025) || die "Could not start client: $@\n";
print $client->name();
sleep(1);

my $mech = new WWW::Mechanize( agent => "Boing_OSC 0.1");
my $aref;
my @osc_out;

$mech->get("http://www.boingboing.net");
if ($mech->success()) {

	foreach ($mech->find_all_links()) {
		my $t = $_->text();
		my $u = $_->url();
		my $b = $_->base();
		last if ($t =~ m/\#boingboing/);

		if (($b =~ m/boingboing\.net/) && ($u =~ m/2008/)) {
			my @l = split(/\n/, $t);
			foreach (@l) {
				next if m/\[IMG\]/;
				next if m/Disc/;
				next if m/perm/;
				next if m/^Link/;
				push(@osc_out, [$_, $u]);
			}
		}
	}

} else {

	print $mech->response->content;

}

my $res;
my $c = 0;
foreach (@osc_out) {
	
	print "sending text: " . $_->[0] . "\n";
	$res = $client->send(["/text", "s", $_->[0]]);
	usleep(100000);
	$res = $client->send(["/url", "s", $_->[1]]);
}
