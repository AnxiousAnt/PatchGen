#!/usr/bin/tclsh

wm geometry .  600x400

tk::canvas .canvas -bg lightblue
.canvas create rectangle 100 50 500 350 -fill green
pack .canvas -side left -expand 1 -fill both

proc drawarray {} {
    set coords {}
    set numpoints 400
    for {set i 0} {$i < $numpoints} {incr i} {
        set position [expr int(400 / ($numpoints - 1) * $i) + 100 ]
        lappend coords $position
        set value [expr { int((300 * rand()) + 50) }]
        lappend coords $value    
    }
    #puts "coords $coords"
    .canvas coords myline $coords
    after idle [list after 10 drawarray]
}

.canvas create line 100 100 200 200 -smooth bezier -tags myline
after idle [list after 10 drawarray]
