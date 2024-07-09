#!/usr/bin/wish

puts "linespace\tascent\t\tdescent\t\tfont name"
puts "---------------------------------------------------------------"
foreach fontname [font families] {
    set metrics [font metrics [list $fontname]]
    set fixed [lindex $metrics end]
    if {$fixed} {
        set ascent [lindex $metrics 1]
        set descent [lindex $metrics 3]
        set linespace [lindex $metrics 5]
        puts "$linespace\t\t$ascent\t\t$descent\t\t$fontname"
    }
}
exit
