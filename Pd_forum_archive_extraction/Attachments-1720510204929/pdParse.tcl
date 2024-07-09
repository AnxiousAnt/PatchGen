#!/usr/bin/env tclsh
#
# pdParse.tcl - Parses pd patch files. The goal it to make editing 
# possible. Based on 
# http://puredata.info/Members/pierpower/Favorites/fileformat
# fjkraan@xs4all.nl, 2014-12-16
# version 0.3

# pdParse.tcl currently parses a Pd patch and puts each line in a list
# with an 'item-locator' prefixed. The item locator is the combination
# of the patch number and object number, separated by a dot. Non-object
# items have "--" as object number, as they do not count. Some 
# indentation is added, but this is just presentation. 
# The list is created to make a future streaming editor for pd-patches
# possible, as removing objects and fix the connections can now be done
# on a line-by-line basis.

# ToDo: - get rid of the 'global lines lines' as it shouldn't be needed.

if {$argc != 1} {
    error "Usage: pdParse.tcl fileName"
}
set file   [open [lindex $argv 0] r]

proc fixRestoreObjNumber {lines objCount} {
	# fixes the object number. The #X restore is detected in the 
	# subpatch but it should have an object number from one level
	# higher. Hence this kludge.
	set lastLine [lindex $lines end]
	if {[regexp -- "\#X restore" $lastLine]} {
		regsub {\?\?} $lastLine $objCount lastLine
	}
	return $lastLine
}

proc parseCanvas {parentNo oldSpaces} {
	set spaces " $oldSpaces"
	global file file
	global lines lines
	global number number
	incr number 
	set myNumber $number
	set objCount 0
	while {[gets $file line] >= 0} {
	    # constructing complete lines
	    set lastChar [string index $line end]
	    while {![string equal $lastChar \;]} {
		set lineAppendix ""
		gets $file lineAppendix
		set line "$line\n$lineAppendix"
		set lastChar [string index $line end]
	    } 
	    # parsing the lines
	    # end sub patch handler, a #X special case
	    if {[regexp -- "\#X restore" $line]} {
		lappend lines "${parentNo}.??: $line"
#		puts "${parentNo}${spaces}??: $line"
		return $lines
	    }
	    # start sub patch handler
	    if {[regexp -- \#N $line]} {
		lappend lines "${myNumber}.--: $line"
#		puts "${myNumber}$spaces< --: $line >"
		set lines [parseCanvas $myNumber $spaces]
		set newLine [fixRestoreObjNumber $lines $objCount]
		set lines [lreplace $lines end end $newLine]
		incr objCount
		continue
	    }
	    # normal object lines handler
	    if {[regexp -- \#X $line]} {
		if { [regexp -- {\#X connect} $line] } {
			lappend lines "${myNumber}.--: $line"
#			puts "${myNumber}$spaces< --: $line >"
		} else {
			lappend lines "${myNumber}.$objCount: $line"
#			puts "${myNumber}$spaces< $objCount: $line >"
			incr objCount
		}
	    }
	    # array contents handler
	    if {[regexp -- \#A $line]} {
		lappend lines "${myNumber}.--: $line"
#		puts "${myNumber}$spaces< --: $line >"
	    }
	}
	return $lines
}

proc addSpace { spaces } {
	return "$spaces "
}

proc removeSpace { spaces } {
	return [string replace $spaces end end]
}

set number -1

set lines [parseCanvas $number ""]

#puts ""
#puts "list size: [llength $lines]"
#puts ""

set spaces ""

# list items from $lines. format <patchNo>.<objectNo>: <objectSpecification> 
foreach item  $lines {
	if {[regexp -- "\#X restore" $item]} {
		set spaces [removeSpace $spaces]
	}
	puts "${spaces}$item"
	if {[regexp -- "\#N canvas" $item]} {
		set spaces [addSpace $spaces]
	}
}
