#!/usr/bin/perl

# WARNING!  This script is really ugly!

use strict;
use warnings;

my $line = "";
my @lines;
my $lineCount = 0;
my $column;
my $lastColumn = 0;
my $printText = "";

my %classnames = ();

my $libraryName = "";
my $objectclassName = "";
my $fileName = "";

my $pageName = "";
my $abbreviationName = "";
my $descriptionName = "";
my $categoryName = "";
my $datatypeName = "";

#------------------------------------------------------------------------------#
# PARSE HELP FILES
#------------------------------------------------------------------------------#

foreach (`/sw/bin/find /Users/hans/Desktop/TODO/wiki_files_hacked/5.reference/ -type f -name '*.pd'`) {
  chop;
  $fileName = "";
  if( (m|.*/5\.reference/([a-zA-Z0-9_-]+)/(.+)-help\.pd|) ||  (m|.*/5\.reference/([a-zA-Z0-9_-]+)/(.+)\.pd|) ) {
#	 print("$1 , $2\t");
	 if( $1 eq 'zflatspace' ) { $libraryName = "flatspace";}
	 else {$libraryName = lc($1); }
	 $objectclassName = $2;
	 $fileName = $_;
  }

  if($fileName) {
	 if( $classnames{$objectclassName} ) {
		$pageName = "${objectclassName}_(${libraryName})";
	 } else {
		$pageName = "${objectclassName}";
	 }
	 open(OBJECTCLASS, ">${libraryName}%2F${objectclassName}.txt");
	 print(OBJECTCLASS "#REDIRECT [[${pageName}]]\n");
	 close(OBJECTCLASS);

	 $classnames{$objectclassName} = 1;
  }
}
  
