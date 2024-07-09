#!/usr/bin/tclsh

wm geometry .  600x400

tk::canvas .canvas -bg lightblue
.canvas create rectangle 100 50 500 350 -fill green
pack .canvas -side left -expand 1 -fill both

proc drawarray {} {
    .canvas delete myline
    set coords {}
    set numpoints 400
    for {set i 0} {$i < $numpoints} {incr i} {
        set position [expr int(400 / ($numpoints - 1) * $i) + 100 ]
        lappend coords $position
        set value [expr { int((300 * rand()) + 50) }]
        lappend coords $value    
    }
    #puts "coords $coords"
    .canvas create line $coords -smooth bezier -tags myline
    after idle [list after 10 drawarray]
}

after idle [list after 10 drawarray]
