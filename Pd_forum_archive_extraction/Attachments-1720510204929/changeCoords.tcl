#!/usr/bin/env tclsh
#

if {$argc < 4} {
    error "Usage: tclsh changeCoords.tcl patchName xcoord ycoord line1 ?line2? ..."
}

set patchName [lindex $argv 0]
set xcoord    [lindex $argv 1]
set ycoord    [lindex $argv 2]
set lineArgCount $argc
set lineList {}

#puts "patchName $patchName; xcoord $xcoord; ycoord $ycoord lineArgCount; $lineArgCount"
# line arguments may be numeric or a numeric range "nn-mm" (inclusive)

for {set start 3} {$lineArgCount > $start} {incr start} {
        set arg [lindex $argv $start]
        if [regexp {(\d+)-(\d+)} $arg lineRange startLine endLine] {
                if {$startLine < $endLine} {
                        for {set line $startLine} {$line <= $endLine} {incr line} {
                                lappend lineList $line
                        }
                }
        } else {
                lappend lineList $arg
        }
}

# puts "lines to patch: $lineList"

set lineCount 1
set f [open $patchName]
while {[gets $f patchLine] >= 0} {
    if [regexp {\#[ANX]} $patchLine] {
        if {[lsearch $lineList $lineCount] < 0} {
            if [regexp {\#X obj (\d+) (\d+) (.+)} $patchLine allOfLine orgX orgY restOfLine] {
                puts "#X obj $xcoord $ycoord $restOfLine"
            } else {
                puts $patchLine
            }
        } else {
            puts $patchLine
        }
        incr lineCount
    } else {
        puts $patchLine
        incr lineCount
    }
}
close $f