#!/usr/bin/wish

puts "------------------------------------------------------------------------"
puts "Tcl/Tk version [info patchlevel]"
puts "------------------------------------------------------------------------"
puts "linespace\twidth\t\tascent\t\tdescent\t\tfont name"
puts "------------------------------------------------------------------------"
foreach fontname [font families] {
    set metrics [font metrics [list $fontname]]
    set width [font measure [list $fontname] m]
    set fixed [lindex $metrics end]
    if {$fixed} {
        set ascent [lindex $metrics 1]
        set descent [lindex $metrics 3]
        set linespace [lindex $metrics 5]
        puts "$linespace\t\t$width\t\t$ascent\t\t$descent\t\t$fontname"
    }
}
#exit
