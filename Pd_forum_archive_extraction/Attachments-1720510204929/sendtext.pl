#!/usr/bin/perl

use IO::Socket;

$socket = new IO::Socket::INET (PeerAddr => "localhost",
                                PeerPort =>  3000,
                                Proto    => "tcp",
                                Type     => SOCK_STREAM)
                                or die "Can't connect $!\n";
if (@ARGV == 0) {
    print "Enter filename:";
    chomp($fname = <STDIN>);
}
else {
   $fname = $ARGV[0];
}
open (FPTR,$fname) || die "Can't Open File: $fname\n";
while ($ligne = <>){

@ascii = unpack("C*", $ligne);    
foreach $val (@ascii) {
   print $socket "$val ";
   }
 }
 print $socket ";"; 

