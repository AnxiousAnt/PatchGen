use strict;
use warnings;

my $PI = 3.14159265;

my $coefs = &lowpass(100, 1, 48000);
print join(", ",  @$coefs ) . "\n";
 
sub lowpass {
  my $f = shift;
  my $bw = shift;
  my $sr = shift;
  
  my $w0 = &wo($f, $sr);
  my $alpha = &alpha($w0, $bw);
  
  print "w0: $w0\n";
  print "alpha: $alpha\n";

  my $b0 = (1.0 - cos($w0))/2;
  my $b1 = 1.0 - cos($w0);
  my $b2 = $b0;
  my $a0 = 1.0 + $alpha;
  my $a1 = -2.0 * cos($w0);
  my $a2 = 1.0 - $alpha;
  
  return [-$a1/$a0, -$a2/$a0, $b0/$a0, $b1/$a0, $b2/$a0];
}

sub sinh {
  my $num = shift;
  return (exp($num)-exp(-$num)) * 0.5;
}

sub ln {
  my ($x) = shift;
  return log($x)/log(2.71828183);
}

sub wo {
  my ($fo, $fs) = @_;
  die "wo vals undefined" unless ($fo && $fs);
  return (2 * $PI * $fo)/$fs;
}

sub alpha {
  my ($w0, $bw) = @_;
  return sin($w0) / &sinh( (&ln(2)/2) * $bw * ($w0 / sin($w0)) );
}
