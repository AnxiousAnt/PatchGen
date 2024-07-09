# plugin to allow searching all the documentation using a regexp
# check the Help menu for the Search item to use it

# Bugs:
# tiny text in combobox dropdown menu on Windows
# can't interrupt long searches on Windows (never get them in Fedora 15)
# Todo:
# try to clean up user input prevent regex error messages

package require Tk 8.5
package require pd_bindings
package require pd_menucommands

namespace eval ::dialog_search:: {
    variable searchtext {}
    variable search_history {}
    variable count {}
    variable genres [list [_ "All documents"] \
			[_ "Object Help Patches"] \
			[_ "All About Pd"] \
			[_ "Tutorials"] \
			[_ "Manual"] \
			[_ "Uncategorized"]
		]
}

# TODO check line formatting options

# find_doc_files
# basedir - the directory to start looking in
proc ::dialog_search::find_doc_files { basedir } {
    # Fix the directory name, this ensures the directory name is in the
    # native format for the platform and contains a final directory seperator
    set basedir [string trimright [file join $basedir { }]]
    set fileList {}

    # Look in the current directory for matching files, -type {f r}
    # means ony readable normal files are looked at, -nocomplain stops
    # an error being thrown if the returned list is empty
    foreach fileName [glob -nocomplain -type {f r} -path $basedir $helpbrowser::doctypes] {
        lappend fileList $fileName
    }

    # Now look for any sub direcories in the current directory
    foreach dirName [glob -nocomplain -type {d  r} -path $basedir *] {
        # Recusively call the routine on the sub directory
	# (if it's not already in Pd's search path) and
	# append any new files to the results
	set nomatch [lsearch [concat [file join $::sys_libdir doc] $::sys_searchpath $::sys_staticpath] $dirName]
	if { $nomatch eq "-1" } {
            set subDirList [find_doc_files $dirName]
            if { [llength $subDirList] > 0 } {
                foreach subDirFile $subDirList {
                    lappend fileList $subDirFile
                }
            }
	}
    }
    return $fileList
}

# TODO: break up into: l
proc ::dialog_search::open_file { xpos ypos mytoplevel clicked } {
    set textwidget "$mytoplevel.resultstext"
    set i [$textwidget index @$xpos,$ypos]
    set range [$textwidget tag nextrange filename $i]
    set filename [eval $textwidget get $range]
    set range [$textwidget tag nextrange basedir $i]
    set basedir [eval $textwidget get $range]
    append basedir "/"
    if {$clicked eq "1"} {
        if {$filename ne ""} {
    	    menu_doc_open $basedir $filename
        }
    } else {
        $mytoplevel.statusbar configure -text "Open $basedir$filename"
    }
}

# only does keywords for now-- maybe expand this to handle any meta tags
proc ::dialog_search::grab_metavalue { xpos ypos mytoplevel clicked } {
    set textwidget "$mytoplevel.resultstext"
#    set xpos_offset 20 
#    set xpos [expr {$xpos + $xpos_offset}]
    set i [$textwidget index @$xpos,$ypos]
    set range [$textwidget tag nextrange metavalue_h $i]
    set value [eval $textwidget get $range]
    set text {keywords.*}
    append text $value
    if {$clicked eq "1"} {
        set ::dialog_search::searchtext ""
        set ::dialog_search::searchtext $text
        ::dialog_search::search
    } else {
        $mytoplevel.statusbar configure -text $text
    }
}

# show/hide results based on genre
proc ::dialog_search::filter_results { combobox text } {
    variable genres
    set elide {}
    if { [$combobox current] eq "0" } {
    	foreach genre $genres {
    	    $text tag configure [join $genre "_"] -elide off
    	    set tag [join $genre "_"]
    	    append tag "_count"
    	    $text tag configure $tag -elide on
	}
	set tag [join [lindex $genres 0] "_"]
	append tag "_count"
	$text tag configure $tag -elide off
    } else {
    	foreach genre $genres {
    	    if { [$combobox get] ne $genre } {
    		$text tag configure [join $genre "_"] -elide on
    		set tag [join $genre "_"]
    		append tag "_count"
    		$text tag configure $tag -elide on
    	    } else {
    		$text tag configure [join $genre "_"] -elide off
    		set tag [join $genre "_"]
    		append tag "_count"
    		$text tag configure $tag -elide off
    	    }
    	}
    }
    $combobox selection clear
    focus $text
}

proc ::dialog_search::readfile {filename} {
    set fp [open $filename]
    set file_contents [split [read $fp] \n]
    close $fp
    return $file_contents
}

proc ::dialog_search::search { } {
    variable searchtext
    variable search_history
    if {$searchtext eq ""} return
    if { [lsearch $search_history $searchtext] eq "-1" } {
    	lappend search_history $searchtext
    	.search.searchtextentry configure -values $search_history
    }
    .search.searchtextentry selection clear
    .search.searchtextentry configure -foreground gray -background gray90
    .search.resultstext configure -state normal
    .search.resultstext delete 0.0 end
    update idletasks
    do_search
    # BUG? the above might cause weird bugs, so consider http://wiki.tcl.tk/1255
    # and http://wiki.tcl.tk/1526
    # after idle [list after 10 ::dialog_search::do_search]
}

proc ::dialog_search::do_search { } {
    variable searchtext
    variable count
    variable genres
    set count {}
    foreach genre $genres {
    	lappend count "0"
    }
    set widget .search.resultstext
    # Get rid of pesky leading/trailing spaces...
    set formatted_searchtext [string trim $searchtext]
    # ...and double spaces between terms
    regsub -all {\s+} $formatted_searchtext { } formatted_searchtext
    foreach basedir [concat [file join $::sys_libdir doc] $::sys_searchpath $::sys_staticpath] {
        # Fix the directory name, this ensures the directory name is in the
        # native format for the platform and contains a final directory seperator
        set basedir [file normalize $basedir]
        foreach docfile [find_doc_files $basedir] {
            searchfile $formatted_searchtext [readfile $docfile] $widget \
                [string replace $docfile 0 [string length $basedir]] $basedir
        }
    }
    .search.searchtextentry configure -foreground black -background white
    $widget insert 0.0 " matching docs.\n"
    set i 0
    foreach genre $genres {
    		set tag [join $genre "_"]
    		append tag "_count"
    		$widget insert 0.0 [lindex $count $i] $tag
    		incr i
    }
    $widget insert 0.0 "       Found "
    $widget insert 0.0 "Home" "link intro"
    $widget configure -state disabled
}

proc ::dialog_search::searchfile {searchtext file_contents widget filename basedir} {
    variable count
    variable genres
    set match 0
    set description ""
    set keywords ""
    set genre ""
    set metadata ""
    set title ""
    set terms_to_match 1
    if { $::dialog_search::matchall == 1 } {
	set terms_to_match [llength $searchtext]
    }
    if {[regexp -nocase -- ".*-help\.pd" $filename]} {
    	# object help
    	set genre 1
    	regsub -nocase -- "(^.*)(5.reference/)(.*)-help.pd" $filename {\1\3} title
    } elseif {[regexp -nocase -- "all_about_.*\.pd" $filename]} {
	regsub -nocase -- ".*all_about_(.)(.*)\.pd" $filename {\1\2} title
	regsub -all -- "_" $title " " title
	# all about pd
	set genre 2
    } elseif {[regexp -nocase -- "\.html?" $filename]} {
    	set title $filename
    	# Pd Manual (or some html page in the docs)
    	set genre 4
    } else {
    	set title $filename
    }
    set wb {[\s,\";\.-]}
    foreach term $searchtext {
	set parsefile 1
	if { $::dialog_search::matchwords == 1 } {
	    if {[regexp -nocase -- \
		[join [list $wb $term $wb] ""] $filename] ||
		[regexp -nocase -- $term \
		    [regsub {.*[\/\\](.*)$} $filename {\1}]]} {
		incr match
		set parsefile 0
	    }
	} else {
	    if {[regexp -nocase -- "$term" $filename]} {
		incr match
		set parsefile 0
	    }
	}
	if { $parsefile } {
	    foreach line $file_contents {
		if { $::dialog_search::matchwords == 1 } {
    		    if {[regexp -nocase -- [join [list $wb $term $wb] ""] $line]} {
    			incr match
    			break
    		    }
		} else {
    		    if {[regexp -nocase -- "$term" $line]} {
    			incr match
    			break
    		    }
		}
	    }
	}
    }
    if { $match >= $terms_to_match } {
        set len [llength [split $searchtext]]
    	regexp -nocase -- "#X text \[0-9\]+ \[0-9\]+ description\[:\]? (.*?);.*" $file_contents -> description
    	regsub -all {[{}\\]} $description {} description
    	regexp -nocase -- "#X text \[0-9\]+ \[0-9\]+ keywords\[:\]? (.*?);.*" $file_contents -> keywords
    	regsub -all {[{}]} $keywords {} keywords
    	if {[regexp -nocase -- "#X text \[0-9\]+ \[0-9\]+ genre tutorial" $file_contents]} {
    	    set genre 3
    	}
    	if { $genre eq "" } {
    	    set genre 5
    	}
    	lset count $genre [expr [lindex $count $genre] + 1]
    	set genre [join [lindex $genres $genre] "_"]
    	lset count 0 [expr [lindex $count 0] + 1]    		
    	# print out the match results
    	$widget insert end "$title" "title link $genre"
    	$widget insert end "$filename" filename
    	$widget insert end "$basedir" basedir
    	if { $description eq "" } {
    	    set description "No DESCRIPTION tag."
    	}
    	$widget insert end "\n$description\n" "description $genre"
    	if { $keywords ne "" } {
    	    $widget insert end "Keywords:" "keywords $genre"
	    set i 0
    	    foreach value $keywords {
		set metavalue "metavalue$i"
		incr i
    		$widget insert end " " link
    		$widget insert end $value "$metavalue keywords link $genre"
                # have to make an elided copy for use with "nextrange"
                # since you can't just get the tag's index underneath
                # the damn cursor!!!
                $widget insert end $value metavalue_h
    	    }
    	$widget insert end "\n" $genre
    	}
    }
}

proc ::dialog_search::ok {mytoplevel} {
    # this is a placeholder for the standard dialog bindings
}

proc ::dialog_search::cancel {mytoplevel} {
    wm withdraw .search
}

proc ::dialog_search::open_search_dialog {mytoplevel} {
    if {[winfo exists $mytoplevel]} {
        wm deiconify $mytoplevel
        raise $mytoplevel
    } else {
        create_dialog $mytoplevel
    }
}

# hack to select all because tk's default bindings apparently
# assume the user is going to want emacs shortcuts to write a
# 1980s program inside a ttk::entry widget !?!?!?!?
proc ::dialog_search::sa { widget } {
    $widget selection range 0 end
    break
}

proc ::dialog_search::intro { t } {
    $t configure -state normal
    $t delete 0.0 end
    $t insert end "Pure Data Documentation Search\n\n" homepage_title
    $t insert end "Enter search terms above and " homepage_description
    $t insert end "use the dropdown menu to choose a category. " homepage_description
    $t insert end "You can also click on one of the tags below " homepage_description
    $t insert end "to start your search.\n\n" homepage_description
 
    $t insert end "Keywords\n" homepage_title
 
    $t insert end "abstraction" "metavalue1 link keywords"
    $t insert end "abstraction" metavalue_h
    $t insert end " object itself is written in Pure Data\n" def

    $t insert end "abstraction_op" "metavalue2 link keywords"
    $t insert end "abstraction_op" metavalue_h
    $t insert end " object that only makes sense in terms of" def
    $t insert end " abstractions\n" def

    $t insert end "analysis" "metavalue3 link keywords"
    $t insert end "analysis" metavalue_h
    $t insert end " object that does analysis\n" def

    $t insert end "anything_op" "metavalue4 link keywords"
    $t insert end "anything_op" metavalue_h
    $t insert end " store or manipulate an anything\n" def

    $t insert end "array" "metavalue5 link keywords"
    $t insert end "array" metavalue_h
    $t insert end " objects for creating and editing arrays\n" def

    $t insert end "bandlimited" "metavalue6 link keywords"
    $t insert end "bandlimited" metavalue_h
    $t insert end " objects that describe themselves as being" def
    $t insert end "bandlimited\n" def

    $t insert end "block_oriented" "metavalue7 link keywords"
    $t insert end "block_oriented" metavalue_h
    $t insert end " see Matju's definition\n" def

    $t insert end "canvas_op" "metavalue8 link keywords"
    $t insert end "canvas_op" metavalue_h
    $t insert end " object whose behavior only makes sense in terms " def
    $t insert end "of a canvas\n" def

    $t insert end "control" "metavalue9 link keywords"
    $t insert end "control" metavalue_h
    $t insert end " control rate objects\n" def

    $t insert end "conversion" "metavalue1 link keywords"
    $t insert end "conversion" metavalue_h
    $t insert end " convert from one set of units to another\n" def

    $t insert end "data_structure" "metavalue2 link keywords"
    $t insert end "data_structure" metavalue_h
    $t insert end " objects for creating and managing data structures\n" def

    $t insert end "filter" "metavalue3 link keywords"
    $t insert end "filter" metavalue_h
    $t insert end " object that filters the data\n" def

    $t insert end "GUI" "metavalue4 link keywords"
    $t insert end "GUI" metavalue_h
    $t insert end " objects that provide a graphical user interface\n" def

    $t insert end "list_op" "metavalue1 link keywords"
    $t insert end "list_op" metavalue_h
    $t insert end " object that manipulates or stores a list\n" def

    $t insert end "MIDI" "metavalue2 link keywords"
    $t insert end "MIDI" metavalue_h
    $t insert end " objects that provide MIDI functionality\n" def

    $t insert end "needs_work" "metavalue3 link keywords"
    $t insert end "needs_work" metavalue_h
    $t insert end " help patches under construction\n" def

    $t insert end "network" "metavalue4 link keywords"
    $t insert end "network" metavalue_h
    $t insert end " object that provides access to or sends/receives " def
    $t insert end "data over a network connection.\n" def

    $t insert end "nonlocal" "metavalue1 link keywords"
    $t insert end "nonlocal" metavalue_h
    $t insert end " objects that can make nonlocal connections to " def
    $t insert end "other objects (i.e., communicate with other objects " def
    $t insert end "without wires\n" def

    $t insert end "orphan" "metavalue2 link keywords"
    $t insert end "orphan" metavalue_h
    $t insert end " help patches that can't get accessed by right-clicking" def
    $t insert end {on the corresponding object (like [drawsymbol])} def
    $t insert end "\n" def

    $t insert end "patchfile_op" "metavalue3 link keywords"
    $t insert end "patchfile_op" metavalue_h
    $t insert end " object whose behavior only makes sense in " def
    $t insert end "terms of a patchfile\n" def

    $t insert end "pd_op" "metavalue4 link keywords"
    $t insert end "pd_op" metavalue_h
    $t insert end " object that can report on or manipulate global Pd " def
    $t insert end "operation\n" def

    $t insert end "ramp" "metavalue1 link keywords"
    $t insert end "ramp" metavalue_h
    $t insert end " a ramp\n" def

    $t insert end "random" "metavalue2 link keywords"
    $t insert end "random" metavalue_h
    $t insert end " object outputs a random value, list, or signal\n" def

    $t insert end "signal" "metavalue3 link keywords"
    $t insert end "signal" metavalue_h
    $t insert end " audiorate objects\n" def

    $t insert end "soundfile" "metavalue4 link keywords"
    $t insert end "soundfile" metavalue_h
    $t insert end " object that can play, manipulate, and/or save a " def
    $t insert end "sound file (wav, ogg, mp3, etc.)\n" def

    $t insert end "storage" "metavalue1 link keywords"
    $t insert end "storage" metavalue_h
    $t insert end " objects whose main function is to store a value\n" def

    $t insert end "symbol_op" "metavalue2 link keywords"
    $t insert end "symbol_op" metavalue_h
    $t insert end " object that manipulates or stores a symbol\n" def

    $t insert end "time" "metavalue3 link keywords"
    $t insert end "time" metavalue_h
    $t insert end " objects that measure time or which the user can " def
    $t insert end "use to manipulate time\n" def

    $t insert end "trigonometry" "metavalue4 link keywords"
    $t insert end "trigonometry" metavalue_h
    $t insert end " objects that provide trigonometric functionality\n" def

    $t configure -state disabled
}

# hack to get <ctrl-backspace> to delete the word to the left of the cursor
proc ::dialog_search::ctrl_bksp {mytoplevel} {
    set last [$mytoplevel index insert]
    set first $last
    while { $first > 0 } {
	set char [string index [$mytoplevel get] $first-1]
	set prev [string index [$mytoplevel get] $first]
	if { $char eq " " && $first < $last && $prev ne " " || [$mytoplevel selection present] } { break }
	incr first -1
    }
    incr first
    $mytoplevel delete $first $last
}

proc ::dialog_search::font_size {text dir} {
    set offset {}
    if {$dir == 1} {
	set offset 2
    } else {
	set offset -2
    }
    foreach tag [$text tag names] {
	set val [$text tag cget $tag -font]
	if {[string is digit -strict [lindex $val 1]]} {
	   $text tag configure $tag -font "Helvetica [expr {max([lindex $val 1]+$offset,8)}]"
	}
    }
}

proc ::dialog_search::create_dialog {mytoplevel} {
    variable selected_file
    variable genres
    toplevel $mytoplevel -class DialogWindow
    wm title $mytoplevel [_ "Search"]

    # style tweak: get rid of arrow so the combobox looks like a simple entry widget
    ttk::style configure Entry.TCombobox -arrowsize 0
    ttk::style configure Genre.TCombobox

    # widgets
    ttk::combobox $mytoplevel.searchtextentry -textvar ::dialog_search::searchtext \
	-font "Helvetica 12" -style "Entry.TCombobox" -cursor "xterm"
    ttk::button $mytoplevel.searchbutton -text [_ "Search"] -takefocus 0 \
	-command ::dialog_search::search
    ttk::frame $mytoplevel.f -padding 2
    ttk::combobox $mytoplevel.f.genrebox -values $genres -state readonly -style "Genre.TCombobox"
    $mytoplevel.f.genrebox current 0
    ttk::checkbutton $mytoplevel.f.matchall_check -text [_ "Match all terms"] \
	-variable ::dialog_search::matchall
    ttk::checkbutton $mytoplevel.f.matchwords_check -text [_ "Match whole words only"] \
	-variable ::dialog_search::matchwords
    ttk::label $mytoplevel.advancedlabel -text [_ "Advanced"] -foreground "#0000ff"
    text $mytoplevel.resultstext -yscrollcommand "$mytoplevel.yscrollbar set" \
        -bg white -highlightcolor blue -height 30 -wrap word -state disabled \
	-padx 8 -pady 4 
    ttk::scrollbar $mytoplevel.yscrollbar -command "$mytoplevel.resultstext yview" \
        -takefocus 0
    ttk::label $mytoplevel.statusbar -text "Pure Data Search" -justify left \
        -padding {4 4 4 4}

    grid $mytoplevel.f.genrebox $mytoplevel.f.matchall_check \
	$mytoplevel.f.matchwords_check -padx 3
    grid $mytoplevel.searchtextentry -column 0 -columnspan 3 -row 0 -padx 3 -pady 2 -sticky ew
    grid $mytoplevel.searchbutton -column 3 -columnspan 2 -row 0 -padx 3 -sticky ew
    grid $mytoplevel.f -column 0 -columnspan 3 -row 1 -sticky ew
    grid $mytoplevel.advancedlabel -column 3 -columnspan 2 -row 1 -sticky w
    grid $mytoplevel.resultstext -column 0 -columnspan 4 -row 2 -sticky nsew
    grid $mytoplevel.yscrollbar -column 4 -row 2 -sticky ns
    grid $mytoplevel.statusbar -column 0 -columnspan 4 -row 3 -sticky nsew
    grid columnconfigure $mytoplevel 0 -weight 1
    grid columnconfigure $mytoplevel 4 -weight 0
    grid rowconfigure    $mytoplevel 2 -weight 1

    # tags
    $mytoplevel.resultstext tag configure title -foreground "#0000ff" -underline on \
	-font "helvetica 14" -spacing1 5
    $mytoplevel.resultstext tag configure filename -elide on
    $mytoplevel.resultstext tag configure metavalue -foreground "#0000ff"
    $mytoplevel.resultstext tag configure metavalue_h -elide on
    $mytoplevel.resultstext tag configure basedir -elide on
    $mytoplevel.resultstext tag configure description -font "helvetica 12"
    $mytoplevel.resultstext tag configure homepage_title -font "helvetica 12" -justify center
    $mytoplevel.resultstext tag configure homepage_description -font "helvetica 12"
    $mytoplevel.resultstext tag configure keywords \
	-font "helvetica 12"
    $mytoplevel.resultstext tag configure def -font "helvetica 10"
    # create a bunch of link bindings so that you get <Enter> and <Leave>
    # events when two links are right next to each other
    $mytoplevel.resultstext tag configure link -foreground "#0000ff"
    $mytoplevel.resultstext tag bind link <Enter> "$mytoplevel.resultstext configure \
        -cursor hand2"
    $mytoplevel.resultstext tag bind link <Leave> "$mytoplevel.resultstext configure \
        -cursor xterm; $mytoplevel.statusbar configure -text \"\""
    $mytoplevel.resultstext tag bind intro <Button-1> " ::dialog_search::intro \
	$mytoplevel.resultstext "
    # hack to force new <Enter> events for tags next to each other
    for {set i 0} {$i<30} {incr i} {
	$mytoplevel.resultstext tag bind "metavalue$i" <Button-1> \
	    "::dialog_search::grab_metavalue %x %y $mytoplevel 1"
        $mytoplevel.resultstext tag bind "metavalue$i" <Enter> \
	    "::dialog_search::grab_metavalue %x %y $mytoplevel 0"
    }
    $mytoplevel.resultstext tag bind title <Button-1> " ::dialog_search::open_file %x %y \
	$mytoplevel 1"
    $mytoplevel.resultstext tag bind title <Enter> "::dialog_search::open_file %x %y \
        $mytoplevel 0"

    # search window widget bindings
    bind $mytoplevel <$::modifier-equal> "::dialog_search::font_size $mytoplevel.resultstext 1"
    bind $mytoplevel <$::modifier-minus> "::dialog_search::font_size $mytoplevel.resultstext 0"
    bind $mytoplevel.searchtextentry <Return> "$mytoplevel.searchbutton invoke"
    bind $mytoplevel.searchtextentry <$::modifier-Key-BackSpace> \
	"::dialog_search::ctrl_bksp $mytoplevel.searchtextentry"
    bind $mytoplevel.searchtextentry <$::modifier-Key-a> \
        "$mytoplevel.searchtextentry selection range 0 end; break"
    bind $mytoplevel.f.genrebox <<ComboboxSelected>> "::dialog_search::filter_results \
	$mytoplevel.f.genrebox $mytoplevel.resultstext"
    bind $mytoplevel.f.matchall_check <Return> "$mytoplevel.searchbutton invoke"
    bind $mytoplevel.f.matchwords_check <Return> "$mytoplevel.searchbutton invoke"
    bind $mytoplevel.advancedlabel <Enter> "$mytoplevel.advancedlabel configure \
	-cursor hand2 "
    bind $mytoplevel.advancedlabel <Leave> "$mytoplevel.advancedlabel configure \
	-cursor xterm "
    bind $mytoplevel.advancedlabel <Button-1> \
	{menu_doc_open doc/5.reference all_about_finding_objects.pd}
    ::pd_bindings::dialog_bindings $mytoplevel "search"

    # Add state and set focus
    $mytoplevel.searchtextentry insert 0 "Enter search terms"
    $mytoplevel.searchtextentry selection range 0 end
    # go ahead and set tags for the default genre
    filter_results $mytoplevel.f.genrebox $mytoplevel.resultstext
    focus $mytoplevel.searchtextentry
    ::dialog_search::intro $mytoplevel.resultstext
    set ::dialog_search::matchall 1
    set ::dialog_search::matchwords 1
}

# create the menu item on load
set mymenu .menubar.help
set inserthere [$mymenu index [_ "Report a bug"]]
$mymenu insert $inserthere separator
$mymenu insert $inserthere command -label [_ "Search"] \
    -command {::dialog_search::open_search_dialog .search}
