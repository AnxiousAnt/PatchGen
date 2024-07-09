# plugin to allow searching all the documentation using a regexp
# check the Help menu for the Search item to use it

package require Tk 8.5
package require pd_bindings
package require pd_menucommands

namespace eval ::dialog_search:: {
    variable searchtext {}
    variable count {}
    variable search_history {}
    variable history_position 0
    variable object_state {1}
    variable all_about_state {1}
    variable tutorial_state {1}
    variable other_state {1}

}

proc ::dialog_search::get_history {direction textwidget} {
    variable search_history
    variable history_position

    incr history_position $direction
    if {$history_position < 0} {set history_position 0}
    if {$history_position > [llength $search_history]} {
        set history_position [llength $search_history]
    }
    $textwidget delete 0 end
    $textwidget insert 0 [lindex $search_history end-[expr $history_position - 1]]
    $textwidget selection range 0 end
}

# TODO search type pulldown menu: object, message, comment, array, any
# TODO search filenames also
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
        # Recusively call the routine on the sub directory and append any
        # new files to the results
        set subDirList [find_doc_files $dirName]
        if { [llength $subDirList] > 0 } {
            foreach subDirFile $subDirList {
                lappend fileList $subDirFile
            }
        }
    }
    return $fileList
}

proc ::dialog_search::open_file { xpos ypos textwidget } {
    set i [$textwidget index @$xpos,$ypos]
    set range [$textwidget tag prevrange filename $i]
    set filename [eval $textwidget get $range]
    set range [$textwidget tag nextrange basedir $i]
    set basedir [eval $textwidget get $range]
    pdtk_post "Basedir is $basedir and file is $filename\n"
    append basedir "/"
    if {$filename ne ""} {
	menu_doc_open $basedir $filename
    }
}

# only does keywords for now-- should make it handle any meta tags
proc ::dialog_search::grab_metavalue { xpos ypos textwidget } {
    # offset y to correct for tag indention-- not sure why it has to be so 
    # large...
    set xpos_offset 20
    set xpos [expr {$xpos + $xpos_offset}]
    set i [$textwidget index @$xpos,$ypos]
    set range [$textwidget tag prevrange metavalue $i]
    set value [eval $textwidget get $range]
    set text {keywords.*}
    append text $value
    set ::dialog_search::searchtext ""
    set ::dialog_search::searchtext $text
    search
}

proc ::dialog_search::showhide { state } {
    if { $state eq "1" } {
	return {off}
    } else {
	return {on}
    }
}

# show/hide results based on genre
proc ::dialog_search::filter_results { textwidget } {
    variable all_about_state
    variable object_state
    variable tutorial_state
    variable other_state
    $textwidget tag configure objectclass -background "#c4dcdc" \
	-elide [::dialog_search::showhide $object_state]
    $textwidget tag configure all_about_pd -background "#fcbcc4" \
	-elide [::dialog_search::showhide $all_about_state]
    $textwidget tag configure tutorial -background "#fcc048" \
	-elide [::dialog_search::showhide $tutorial_state]
    $textwidget tag configure other -background "#ffffff" \
	-elide [::dialog_search::showhide $other_state]

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
    pdtk_post "Appended $searchtext to history\n"
    .search.searchtextentry selection clear
    .search.searchtextentry configure -foreground gray -background gray90
    .search.resultstext configure -state normal
    .search.resultstext delete 0.0 end
    update idletasks
    do_search
    # BUG? the above might cause weird bugs, so consider http://wiki.tcl.tk/1255
    # and http://wiki.tcl.tk/1526
#    after idle [list after 10 ::dialog_search::do_search]
}

proc ::dialog_search::do_search { } {
    variable searchtext
    variable count
    set count 0
    set widget .search.resultstext
    foreach basedir [concat [file join $::sys_libdir doc] $::sys_searchpath $::sys_staticpath] {
        # Fix the directory name, this ensures the directory name is in the
        # native format for the platform and contains a final directory seperator
        set basedir [file normalize $basedir]
        foreach docfile [find_doc_files $basedir] {
            searchfile $searchtext [readfile $docfile] $widget \
                [string replace $docfile 0 [string length $basedir]] $basedir
        }
    }
    .search.searchtextentry configure -foreground black -background white
    $widget insert 0.0 "       Found $count matching docs.\n"
    $widget insert 0.0 "Home" "link intro"
    $widget configure -state disabled
}

proc ::dialog_search::searchfile {searchtext file_contents widget filename basedir} {
    variable count
    set match 0
    set description ""
    set keywords ""
    set matchingtext ""
    set genre ""
    set metadata ""
#    set searchtext [regsub -all { } $searchtext {.*}]
    if {[regexp -nocase -- ".*-help.pd" $filename]} {
	set genre "objectclass"
    }
    if {[regexp -nocase -- "\[^a-zA-Z\]$searchtext" $filename]} {
	set match [llength $searchtext]
    } else {
	set terms [split $searchtext]
	foreach term $terms {
	    foreach line $file_contents {
		if {[regexp -nocase -- "$term" $line]} {
		    incr match
		    break
		}
	    }
	}
    }
    if { $match eq [llength [split $searchtext]] } {
	set len [llength [split $searchtext]]
	pdtk_post "Length is $len. Searchtext is $searchtext.\n"
	incr count
	regexp -nocase -- "#X text \[0-9\]+ \[0-9\]+ description\[:\]? (.*?);.*" $file_contents -> description
	regsub -all {[{}]} $description {} description
	regexp -nocase -- "#X text \[0-9\]+ \[0-9\]+ keywords\[:\]? (.*?);.*" $file_contents -> keywords
	regsub -all {[{}]} $keywords {} keywords
	if {[regexp -nocase -- "#X text \[0-9\]+ \[0-9\]+ genre all_about_pd" $file_contents]} {
	    set genre "all_about_pd"
	}
	if {[regexp -nocase -- "#X text \[0-9\]+ \[0-9\]+ genre tutorial" $file_contents]} {
	    set genre "tutorial"
	}
    }

#   The following would print out the line of the matched text, but I found it
#   ugly and rather unhelpful:
#    foreach line $file_contents {
        # TODO this could be optimized so that the lines are added to
        # a var, then the regsubs are run on the whole text, then its
        # inserted into the widget

#	if { $match ne "1" } {
#            if {[regexp -nocase -- "\[^a-zA-Z\]$searchtext" $line]} {
#                set line [regsub { \\} $line {}]
#                set line [regsub {\\} $line {}]
#                set line [regsub {^#X text [0-9]+ [0-9]+ (.*?);*} $line {\1}]
#                set line [regsub {^#X obj [0-9]+ [0-9]+ (.*?);*} $line {［ \1］}]
#                set line [regsub {^#X msg [0-9]+ [0-9]+ (.*?);*} $line {⃒ \1〈}]
#                set line [regsub {^#X (\S+) [0-9]+ [0-9]+(.*?);*} $line {〈\1〉\2}]
#                set matchingtext [regsub {^#N \S+ [0-9]+ [0-9]+ [0-9]+ [0-9]+ (.*?);} \
#                                    $line {[pd \1]}]
#		set match 1
#            }
#	}
#    }

    if { $genre eq "" } {
	set genre "other"
    }
    if { $match eq [llength [split $searchtext]] } {
	$widget insert end "$filename" "filename link $genre"
	$widget insert end "$basedir" basedir
	if { $description eq "" } {
	    set description "No DESCRIPTION tag."
	}
	$widget insert end "\n$description\n" "description $genre"
	if { $keywords ne "" } {
	    $widget insert end "Keywords:" "keywords $genre"
	    foreach value $keywords {
		$widget insert end " " $genre
		$widget insert end $value "metavalue keywords link $genre"
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

proc ::dialog_search::sa { widget } {
    $widget selection range 0 end
    break
}

proc ::dialog_search::intro { t } {
    $t configure -state normal
    $t delete 0.0 end
    $t insert end "Pure Data Documentation Search\n"
    $t insert end "Enter search terms above or use one of the "
    $t insert end "tags below to find help in the form of object "
    $t insert end "class help patches, reference patches all about "
    $t insert end "Pure Data concepts, tutorials, and everything "
    $t insert end "else.\n"
    $t insert end "Tags\n"

    $t insert end "abstraction" "metavalue link keywords"
    $t insert end " object itself is written in Pure Data\n" def

    $t insert end "abstraction_op" "metavalue link keywords"
    $t insert end " object that only makes sense in terms of" def
    $t insert end "abstractions\n" def

    $t insert end "analysis" "metavalue link keywords"
    $t insert end " object that does analysis\n" def

    $t insert end "anything_op" "metavalue link keywords"
    $t insert end " store or manipulate an anything\n" def

    $t insert end "array" "metavalue link keywords"
    $t insert end " objects for creating and editing arrays\n" def

    $t insert end "bandlimited" "metavalue link keywords"
    $t insert end " objects that describe themselves as being" def
    $t insert end "bandlimited\n" def

    $t insert end "block_oriented" "metavalue link keywords"
    $t insert end " see Matju's definition\n" def

    $t insert end "canvas_op" "metavalue link keywords"
    $t insert end " object whose behavior only makes sense in terms " def
    $t insert end "of a canvas\n" def

    $t insert end "control" "metavalue link keywords"
    $t insert end " control rate objects\n" def

    $t insert end "conversion" "metavalue link keywords"
    $t insert end " convert from one set of units to another\n" def

    $t insert end "data_structure" "metavalue link keywords"
    $t insert end " objects for creating and managing data structures\n" def

    $t insert end "filter" "metavalue link keywords"
    $t insert end " object that filters the data\n" def

    $t insert end "GUI" "metavalue link keywords"
    $t insert end " objects that provide a graphical user interface\n" def

    $t insert end "list_op" "metavalue link keywords"
    $t insert end " object that manipulates or stores a list\n" def

    $t insert end "MIDI" "metavalue link keywords"
    $t insert end " objects that provide MIDI functionality\n" def

    $t insert end "needs_work" "metavalue link keywords"
    $t insert end " help patches under construction\n" def

    $t insert end "network" "metavalue link keywords"
    $t insert end " object that provides access to or sends/receives " def
    $t insert end "data over a network connection.\n" def

    $t insert end "nonlocal" "metavalue link keywords"
    $t insert end " objects that can make nonlocal connections to " def
    $t insert end "other objects (i.e., communicate with other objects " def
    $t insert end "without wires\n" def

    $t insert end "orphan" "metavalue link keywords"
    $t insert end " help patches that can't get accessed by right-clicking" def
    $t insert end {on the corresponding object (like [drawsymbol])} def
    $t insert end "\n" def

    $t insert end "patchfile_op" "metavalue link keywords"
    $t insert end " object whose behavior only makes sense in " def
    $t insert end "terms of a patchfile\n" def

    $t insert end "pd_op" "metavalue link keywords"
    $t insert end " object that can report on or manipulate global Pd " def
    $t insert end "operation\n" def

    $t insert end "ramp" "metavalue link keywords"
    $t insert end " a ramp\n" def

    $t insert end "random" "metavalue link keywords"
    $t insert end " object outputs a random value, list, or signal\n" def

    $t insert end "signal" "metavalue link keywords"
    $t insert end " audiorate objects\n" def

    $t insert end "soundfile" "metavalue link keywords"
    $t insert end " object that can play, manipulate, and/or save a " def
    $t insert end "sound file (wav, ogg, mp3, etc.)\n" def

    $t insert end "storage" "metavalue link keywords"
    $t insert end " objects whose main function is to store a value\n" def

    $t insert end "symbol_op" "metavalue link keywords"
    $t insert end " object that manipulates or stores a symbol\n" def

    $t insert end "time" "metavalue link keywords"
    $t insert end " objects that measure time or which the user can " def
    $t insert end "use to manipulate time\n" def

    $t insert end "trigonometry" "metavalue link keywords"
    $t insert end " objects that provide trigonometric functionality\n" def
}

proc ::dialog_search::create_dialog {mytoplevel} {
    variable selected_file
    toplevel $mytoplevel
    wm title $mytoplevel [_ "Search"]

    ttk::combobox $mytoplevel.searchtextentry -textvar ::dialog_search::searchtext \
	-font "Helvetica 12"

    $mytoplevel.searchtextentry insert 0 "Enter search terms"
    $mytoplevel.searchtextentry selection range 0 end

#    entry $mytoplevel.searchtextentry -bg white -textvar ::dialog_search::searchtext \
#        -highlightbackground lightgray \
#        -highlightcolor blue -font 18 -borderwidth 1

    bind $mytoplevel.searchtextentry <Return> "$mytoplevel.searchbutton invoke"

#    bind $mytoplevel.searchtextentry <Up> \
#	"::dialog_search::get_history 1 $mytoplevel.searchtextentry"
#    bind $mytoplevel.searchtextentry <Down> \
#	"::dialog_search::get_history -1 $mytoplevel.searchtextentry"
    bind $mytoplevel.searchtextentry <$::modifier-Key-a> \
        "$mytoplevel.searchtextentry selection range 0 end; break"
    text $mytoplevel.resultstext -yscrollcommand "$mytoplevel.yscrollbar set" \
        -bg white -highlightcolor blue -height 30 -width 60 -wrap word -state disabled
    $mytoplevel.resultstext tag configure filename -foreground "#0000ff" -underline on \
	-font "helvetica 14" -lmargin1 5 -lmargin2 5 -spacing1 5
    $mytoplevel.resultstext tag configure metavalue -foreground "#0000ff"
    $mytoplevel.resultstext tag configure basedir -elide on
    $mytoplevel.resultstext tag configure description -font "helvetica 12" \
	-lmargin1 5 -lmargin2 5
    $mytoplevel.resultstext tag configure keywords \
	-lmargin1 5 -lmargin2 5 -font "helvetica 12"
    $mytoplevel.resultstext tag configure def \
	-lmargin1 5 -lmargin2 5 -font "helvetica 10"
    scrollbar $mytoplevel.yscrollbar -command "$mytoplevel.resultstext yview" \
        -takefocus 0
    button $mytoplevel.searchbutton -text [_ "Search"] -takefocus 0 \
        -background lightgray -highlightbackground lightgray \
        -command ::dialog_search::search

    # Genre tags
    $mytoplevel.resultstext tag configure all_about_pd -background "#fcbcc4"
    $mytoplevel.resultstext tag configure objectclass -background "#c4dcdc"
    $mytoplevel.resultstext tag configure tutorial -background "#fcc048"
    $mytoplevel.resultstext tag configure other

    ttk::frame $mytoplevel.f -width 80

#    ttk::style configure obj.TCheckbutton -background "#c4dcdc"
#    ttk::style configure aapd.TCheckbutton -background "#fcbcc4"
#    ttk::style configure tutorial.TCheckbutton -background "#fcc048"
#    ttk::style configure other.TCheckbutton -background "#ffffff"

    set blah raised

    checkbutton $mytoplevel.f.objectcheck -text "Object Classes" -background "#c4dcdc" \
	-activebackground "#c4dcdc" -variable ::dialog_search::object_state \
	-command "::dialog_search::filter_results $mytoplevel.resultstext" -borderwidth 1 -relief $blah
    checkbutton $mytoplevel.f.all_aboutcheck -text "All About Pd" -background "#fcbcc4" \
	-activebackground "#fcbcc4" -variable ::dialog_search::all_about_state \
	-command "::dialog_search::filter_results $mytoplevel.resultstext" -borderwidth 1 -relief $blah
    checkbutton $mytoplevel.f.tutorialcheck -text "Tutorials " -background "#fcc048" \
	-activebackground "#fcc048" -variable ::dialog_search::tutorial_state \
	-command "::dialog_search::filter_results $mytoplevel.resultstext" -borderwidth 1 -relief $blah
    checkbutton $mytoplevel.f.othercheck -text "Other   " -background "#ffffff" \
	-activebackground "#ffffff" -variable ::dialog_search::other_state \
	-command "::dialog_search::filter_results $mytoplevel.resultstext" -borderwidth 1 -relief $blah


    grid $mytoplevel.f.objectcheck $mytoplevel.f.all_aboutcheck $mytoplevel.f.tutorialcheck \
	$mytoplevel.f.othercheck


    label $mytoplevel.advancedlabel -text "Advanced" -foreground "#0000ff"
    bind $mytoplevel.advancedlabel <Enter> "$mytoplevel.advancedlabel configure \
	-cursor hand2 "
    bind $mytoplevel.advancedlabel <Leave> "$mytoplevel.advancedlabel configure \
	-cursor xterm "
 
    bind $mytoplevel.advancedlabel <Button-1> \
	{menu_doc_open doc/5.reference all_about_finding_objects.pd}
    grid $mytoplevel.searchtextentry -column 0 -row 0 -sticky ew
    grid $mytoplevel.searchbutton -column 1 -columnspan 2 -row 0 -sticky ew
    grid $mytoplevel.f -column 0 -row 1 -sticky ew
    grid $mytoplevel.advancedlabel -column 1 -columnspan 2 -row 1 -sticky e
    grid $mytoplevel.resultstext -column 0 -columnspan 2 -row 2 -sticky news
    grid $mytoplevel.yscrollbar -column 2 -row 2 -sticky nes
    grid columnconfigure $mytoplevel 0 -weight 1
    grid rowconfigure    $mytoplevel 2 -weight 1

    $mytoplevel.resultstext tag configure link -foreground "#0000ff"

    $mytoplevel.resultstext tag bind intro <Button-1> " ::dialog_search::intro \
	$mytoplevel.resultstext "


    $mytoplevel.resultstext tag bind metavalue <Button-1> " ::dialog_search::grab_metavalue %x %y \
	$mytoplevel.resultstext "
    $mytoplevel.resultstext tag bind filename <Button-1> " ::dialog_search::open_file %x %y \
	$mytoplevel.resultstext "
    $mytoplevel.resultstext tag bind link <Enter> " $mytoplevel.resultstext configure \
	-cursor hand2 "
    $mytoplevel.resultstext tag bind link <Leave> " $mytoplevel.resultstext configure \
	-cursor xterm "

   ::pd_bindings::dialog_bindings $mytoplevel "search"
    
    focus $mytoplevel.searchtextentry

    ::dialog_search::intro $mytoplevel.resultstext
}

# create the menu item on load
set mymenu .menubar.help
set inserthere [$mymenu index [_ "Report a bug"]]
$mymenu insert $inserthere separator
$mymenu insert $inserthere command -label [_ "Search"] \
    -command {::dialog_search::open_search_dialog .search}
