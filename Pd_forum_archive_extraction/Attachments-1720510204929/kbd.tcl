
    proc kbd_play { m b x y d} {
	foreach a {{rs {0 sunken 1 raised}} {v {0 0 1 {($y - [winfo rooty $w]) / ([winfo height $w] + 0.0)}}}} {array set [lindex $a 0] [lindex $a 1]}
	set r "-sys_gui-r"
	set rcv $d$r
	if {$b == 0} {pdsend "$rcv release"}
	if {$b == 1} {pdsend "$rcv press"}}



    proc kbd {path octaves b f d} {
	$path config -width [expr $octaves * 66]
	variable _
	set bw {0 1 0 1 0 0 1 0 1 0 1 0} ; set npl {0 1 1 2 2 3 4 4 5 5 6 6}
	foreach a {{bg "0 $b 1 $f"} {fg "1 $b 0 $f"} {an {0 ";lower $wk" 1 "-anchor n"}} {rw {0 {[expr 1 / 7.]} 1 {[expr 1 / 10.]}}} {rh {0 {[expr 1.]} 1 {[expr 6 / 10.]}}}} {array set [lindex $a 0] [lindex $a 1]}
	for {set o 0} {$o < $octaves} {incr o} {
    for {set on 0} {$on < 12} {incr on} {
	set wk $path.[expr $o * 12 + $on] ; set n [lindex $bw $on]
	eval "label $wk -bg $bg($n) -fg $fg($n) -bd 1 -relief raised;place $wk -relx [expr [lindex $npl $on] / 7.] -y 0 -relwidth $rw($n) -relheight $rh($n) $an($n)"
	bind $wk <1> "kbd_play 0 1 %X %Y $d";bind $wk <ButtonRelease-1> "kbd_play 0 0 %X %Y $d"}
#	place $path -relheight 1.0 -relx [expr $o / $octaves.0] -relwidth [expr 1 / $octaves.0]}}


