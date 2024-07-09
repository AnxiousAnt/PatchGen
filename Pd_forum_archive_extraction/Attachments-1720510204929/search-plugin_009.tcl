# plugin to allow searching all the documentation using a regexp
# check the Help menu for the Search item to use it

# Bugs:
# tiny text in combobox dropdown menu on Windows
# can't interrupt long searches on Windows (never get them in GNU/Linux)
# Todo:
# try to clean up user input prevent regex error messages

package require Tk 8.5
package require pd_bindings
package require pd_menucommands

namespace eval ::dialog_search:: {
    variable searchfont [list {DejaVu Sans}]
    variable searchtext {}
    variable search_history {}
    variable count {}
    variable navbar {}
    variable genres [list [_ "All documents"] \
			[_ "Object Help Patches"] \
			[_ "All About Pd"] \
			[_ "Tutorials"] \
			[_ "Manual"] \
			[_ "Uncategorized"] \
    ]
}

# TODO check line formatting options

# find_doc_files
# basedir - the directory to start looking in
proc ::dialog_search::find_doc_files { basedir recursive } {
    # Fix the directory name, this ensures the directory name is in the
    # native format for the platform and contains a final directory seperator
    set basedir [string trimright [file join $basedir { }]]
    set fileList {}

    # Look in the current directory for matching files, -type {f r}
    # means only readable normal files are looked at, -nocomplain stops
    # an error being thrown if the returned list is empty
    foreach fileName [glob -nocomplain -type {f r} -path $basedir $helpbrowser::doctypes] {
        lappend fileList $fileName
    }

    # If a recursive search is specified the look in
    # subdirectories for more files.  This is used for search
    # results and is filtered to make sure we don't list results twice
    # if one of our basedirs happens to be a subdirectory of something
    # in the search patch (i.e., this wouldn't be appropriate for building
    # a file browser
    if {$recursive} {
        foreach dirName [glob -nocomplain -type {d  r} -path $basedir *] {
            # Recusively call the routine on the sub directory
	    # (if it's not already in Pd's search path) and
	    # append any new files to the results
	    set nomatch [lsearch [concat [file join $::sys_libdir doc] \
		$::sys_searchpath $::sys_staticpath] $dirName]
	    if { $nomatch eq "-1" } {
                set subDirList [find_doc_files $dirName 1]
                if { [llength $subDirList] > 0 } {
                    foreach subDirFile $subDirList {
                        lappend fileList $subDirFile
                    }
                }
	    }
	}
    }
    return $fileList
}

# TODO: break up into: l
proc ::dialog_search::open_file { xpos ypos mytoplevel type clicked } {
    set textwidget "$mytoplevel.resultstext"
    set i [$textwidget index @$xpos,$ypos]
    set range [$textwidget tag nextrange filename $i]
    set filename [eval $textwidget get $range]
    set range [$textwidget tag nextrange basedir $i]
    set basedir [file normalize [eval $textwidget get $range]]
    if {$clicked eq "1"} {
	if {$type eq "file"} {
            menu_doc_open $basedir $filename
	} else {
	    menu_doc_open [file dirname [file join $basedir $filename]] {}
	}
    } else {
	$mytoplevel.resultstext configure -cursor hand2
	if {$type eq "file"} {
            $mytoplevel.statusbar configure -text "Open [file join $basedir $filename]"
	} else {
	    set msg ""
	    if {$type eq "dir_in_fm"} {set msg {in external file browser: }}
	    $mytoplevel.statusbar configure -text [format "Browse %s%s" \
		$msg [file dirname [file join $basedir $filename]]]
	}
    }
}

# only does keywords for now-- maybe expand this to handle any meta tags
proc ::dialog_search::grab_metavalue { xpos ypos mytoplevel clicked } {
    set textwidget "$mytoplevel.resultstext"
    set i [$textwidget index @$xpos,$ypos]
    set range [$textwidget tag nextrange metavalue_h $i]
    set value [eval $textwidget get $range]
    set range [$textwidget tag prevrange metakey $i]
    set key [eval $textwidget get $range]
    regsub ":.*" $key {} key
    set key [string tolower $key]
    append text $key {[^;]*} $value {.*?;}
    if {$clicked eq "1"} {
        ::dialog_search::searchfor $text
    } else {
        $mytoplevel.statusbar configure -text [format "Search for pattern: %s" $text]
    }
}

proc ::dialog_search::searchfor {text} {
    set ::dialog_search::searchtext ""
    set ::dialog_search::searchtext $text
    ::dialog_search::search
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
    variable navbar {}
    set count {}
    set filelist {}
    foreach genre $genres {
    	lappend count 0
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
        foreach docfile [find_doc_files $basedir 1] {
	    set docname [string replace $docfile 0 [string length $basedir]]
	    lappend filelist [list "$docname" "$docfile" "$basedir"]
        }
    }
    set filelist [lsort -command searchfile_sort $filelist]
    foreach {docname docfile basedir} [join $filelist] {
	searchfile $formatted_searchtext [readfile $docfile] $widget \
	    $docname $basedir
    }
    .search.searchtextentry configure -foreground black -background white
    print_navbar $widget
    $widget insert 1.end "       Found " "navbar"
    set i 0
    foreach genre $genres {
    		set tag [join $genre "_"]
    		append tag "_count"
    		$widget insert 1.end [lindex $count $i] "$tag navbar"
    		incr i
    }
    $widget insert 1.end " matching docs." "navbar"
    $widget configure -state disabled
}

proc ::dialog_search::searchfile_sort {a b} {
    set pattern {{.*5\.reference[\\\/](.*)}}
    set a0 [regsub {.*5\.reference[\\\/](.*)} [lindex $a 0] { \1}]
    set b0 [regsub {.*5\.reference[\\\/](.*)} [lindex $b 0] { \1}]
    return [string compare -nocase $a0 $b0]
}

proc ::dialog_search::directory_sort {a b} {
    set pattern {(license\.txt|readme\.txt)}
    set a [regsub -nocase $pattern $a {~~~~\1}]
    set b [regsub -nocase $pattern $b {~~~~\2}]
    return [string compare -nocase $a $b]
}

proc ::dialog_search::searchfile {searchtext file_contents widget filename basedir} {
    variable count
    variable genres
    set match 0
    set terms_to_match 1
    if { $::dialog_search::matchall == 1 } {
	set terms_to_match [llength $searchtext]
    }
    # let's design our own word boundaries since tildes often end Pure Data words.
    # homemade left boundary isn't necessary, but makes it easy to change later 
    set lb {(?:^|[^[:alnum:]_])}
    set rb {(?![[:alnum:]_~])}
    foreach term $searchtext {
	set parsefile 1
	if {$::dialog_search::matchwords == 1} {
	    set term [join [list $lb $term $rb] ""]
	}
	if {[regexp -nocase -- $term $filename]} {
	    incr match
	    set parsefile 0
	}
	if { $parsefile } {
    		if {[regexp -nocase -- $term [join $file_contents]]} {
    		    incr match
		}
	}
    }
    if { $match >= $terms_to_match } {
        ::dialog_search::printresult $filename $basedir $file_contents $widget 1
    }
}

proc ::dialog_search::printresult {filename basedir file_contents widget mixed_dirs} {
    variable count
    variable genres
    set description ""
    set keywords ""
    set genre ""
    set metadata ""
    set title ""
    if {[regexp -nocase -- ".*-help\.pd" $filename]} {
    	# object help
    	set genre 1
    	regsub -nocase -- "(?:^|(?:5.reference/))(.*)-help.pd" $filename {\1} title
    } elseif {[regexp -nocase -- "all_about_.*\.pd" $filename]} {
	regsub -nocase -- {(?:.*[/\\])?(.*)\.pd} $filename {\1} title
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
	if {[regexp -nocase {license\.txt} $filename]} {
	    set description "text of the license for this software"
	} elseif {[regexp -nocase {readme\.txt} $filename]} {
	    set description "general information from the author"
	} else {
    	    regexp -nocase -- "#X text \[0-9\]+ \[0-9\]+ description\[:\]? (.*?);.*" $file_contents -> description
    	    regsub -all {[{}\\]} $description {} description
	}
    	regexp -nocase -- "#X text \[0-9\]+ \[0-9\]+ keywords\[:\]? (.*?);.*" $file_contents -> keywords
    	regsub -all {[{}]} $keywords {} keywords
    	if {[regexp -nocase -- "#X text \[0-9\]+ \[0-9\]+ genre tutorial" $file_contents]} {
    	    set genre 3
    	}
    	if { $genre eq "" } {
    	    set genre 5
    	}
    	lset count $genre [expr [lindex $count $genre] + 1]
    	set genre_name [join [lindex $genres $genre] "_"]
    	lset count 0 [expr [lindex $count 0] + 1]    		
    	# print out an entry for the file
    	$widget insert end "$title" "title link $genre_name"
	if {$mixed_dirs} {
	    if { $genre == 1 } {
	        $widget insert end " "
	        $widget image create end -image ::dialog_search::help
	    }
	    $widget insert end " "
	    $widget image create end -image ::dialog_search::folder
	    if { $genre == 1 } {
	        $widget tag add help_icon "end -4indices" "end -3indices"
	        $widget tag add $genre_name "end -5indices" end
	    } else {
	        $widget tag add $genre_name "end -3indices" end
	    }
	    $widget tag add folder_icon "end -2indices" "end -1indices"
	}
	$widget insert end "$basedir" basedir
	$widget insert end "$filename" filename
    	if { $description eq "" } {
    	    set description "No DESCRIPTION tag."
    	}
    	$widget insert end "\n$description\n" "description $genre_name"
    	if { $keywords ne "" } {
    	    $widget insert end "Keywords:" "metakey $genre_name"
	    set i 0
    	    foreach value $keywords {
		set metavalue "metavalue$i"
		incr i
    		$widget insert end " " "link $genre_name"
    		$widget insert end $value "$metavalue keywords link $genre_name"
                # have to make an elided copy for use with "nextrange"
                # since I can't just get the tag's index underneath
                # the damn cursor!!!
                $widget insert end $value metavalue_h
    	    }
    	$widget insert end "\n" $genre_name
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
# assume the user is going to want emacs shortcuts
proc ::dialog_search::sa { widget } {
    $widget selection range 0 end
    break
}

proc ::dialog_search::intro { t } {
    variable navbar {}
    $t configure -state normal
    $t delete 0.0 end
    $t insert end "Pure Data Documentation Search\n" homepage_title
    $t insert end "Enter terms above and " homepage_description
    $t insert end "use the dropdown menu to choose a category.\n" homepage_description
    $t insert end "Introductory Topics\n" homepage_title
    $t insert end "Here are some good starting points if you are new to\
	Pure Data\n" homepage_description
    set intro_docs { \
        "Pd Manual" 1.manual "HTML manual for Pure Data" \
        "Control Structure" 2.control.examples "tutorials for control objects" \
        "Audio Signals" 3.audio.examples "tutorials for audio signals" \
        "Data Structures" 4.data.structures "creating graphical objects in Pure Data" \
    }
    set i 0
    foreach {title dir desc} $intro_docs {
        $t insert end "$title" "link clickable_dir intro_link$i"
        $t insert end [file join $::sys_libdir doc $dir] basedir
        $t insert end "0" is_libdir
        $t insert end "dummy" filename
	$t insert end " $desc\n" description
	set i [expr {($i+1)%30}]
    }
    $t insert end "All About Pd" "link homepage_file"
    $t insert end [file join $::sys_libdir doc 5.reference] basedir
    $t insert end all_about.pd filename
    $t insert end " reference patches for key concepts and settings\
	in Pd\n" description
    $t insert end "External Libraries" "link libdirs intro_libdirs"
    $t insert end " libraries that add functionality to Pd\n" description
    $t insert end "Advanced Topics\n" homepage_title
    set advanced_docs [list \
	"Networking" [file join manuals 3.Networking] "sending data over networks with Pd" \
        "Writing Externals" 6.externs "how to code control and signal objects in C" \
	"Dynamic Patching" [file join manuals pd-msg] "how to construct internal messages and send\
	    to the Pd engine or a canvas" \
	"Implementation Details" [file join manuals Pd] "file format specification, license text,\
	    etc."
    ]
    set i 0
    foreach {title dir desc} $advanced_docs {
	$t insert end "$title" "link clickable_dir intro_link$i"
	$t insert end [file join $::sys_libdir doc $dir] basedir
	$t insert end "0" is_libdir
	$t insert end "dummy" filename
	$t insert end " $desc\n" description
	set i [expr {($i+1)%30}]
    }
    $t insert end "Keywords\n" homepage_title
    $t insert end "Many of the help documents are categorized by keyword. Click a \
	one below to get a list of all documents that have been tagged with it.\n" \
	homepage_description
    set keywords { \
    {abstraction "object itself is written in Pure Data"} \
    {abstraction_op "object's behavior only makes sense inside an abstraction"} \
    {analysis "performance some sort of analysis on the incoming signal or value"} \
    {anything_op "store or manipulate an anything"} \
    {array "create or manipulate an array"} \
    {bandlimited "object describes itself as being bandlimited"} \
    {block_oriented "signal object that performs block-wide operations (as opposed to\
	repeating the same operation for each sample of the block)"} \
    {canvas_op "object's behavior only makes sense in context of a canvas"} \
    {control "control rate objects"} \
    {conversion "convert from one set of units to another"} \
    {data_structure "create or manage data structures"} \
    {filter "object that filters incoming data"} \
    {GUI "graphical user interface"} \
    {list_op "object that manipulates, outputs, or stores a list"} \
    {MIDI "object that provides MIDI functionality"} \
    {needs_work "help patches that are still under construction or\
	otherwise are incomplete"} \
    {network "provides access to or sends/receives data over a network"} \
    {nonlocal "pass messages or data without patch wires"} \
    {orphan "help patches that cannot be accessed by right-clicking \"help\"\
	for the corresponding object"} \
    {patchfile_op "object whose behavior only makes sense in terms of a Pure\
	Data patch"} \
    {pd_op "object that can report on or manipulate global data associated with\
	the running instance of Pd"} \
    {ramp "create a ramp between a starting and ending value"} \
    {random "output a random value, list, signal, or other random data"} \
    {signal "audiorate object (so called \"tilde\" object"} \
    {soundfile "object that can play, manipulate, and/or save a sound file\
	(wav, ogg, flac, mp3, etc.)"} \
    {storage "object whose main function is to store data"} \
    {symbol_op "manipulate or store a symbol"} \
    {time "measure and/or manipulate time"} \
    {trigonometry "provide trogonometric functionality"} \
    }
    set i 0
    foreach keyword $keywords {
        $t insert end "keywords" "metakey hide"
        $t insert end [lindex $keyword 0] "metavalue$i link"
        $t insert end [lindex $keyword 0] metavalue_h
        $t insert end " [lindex $keyword 1]\n" description
        set i [expr {($i+1)%30}]
    }
    $t configure -state disabled
}

# hack to get <ctrl-backspace> to delete the word to the left of the cursor
proc ::dialog_search::ctrl_bksp {mytoplevel} {
    set last [$mytoplevel index insert]
    set first $last
    while { $first > 0 } {
	set char [string index [$mytoplevel get] $first-1]
	set prev [string index [$mytoplevel get] $first]
	if { [regexp {[[:punct:][:space:]\|\^\~\`]} $char] &&
	     $first < $last &&
	     [regexp {[^[:punct:][:space:]\|\^\~\`]} $prev] ||
	     [$mytoplevel selection present] } { break }
	incr first -1
    }
    incr first
    $mytoplevel delete $first $last
}

proc ::dialog_search::font_size {text direction} {
    variable searchfont
    set offset {}
    set min_fontsize 8
    if {$direction == 1} {
	set offset 2
    } else {
	set offset -2
    }
    set update 1
    foreach tag [$text tag names] {
	set val [$text tag cget $tag -font]
	if {[string is digit -strict [lindex $val 1]] &&
	    [expr {[lindex $val 1]+$offset}] < $min_fontsize} {
		set update 0
	    }
    }
    if {$update} {
        foreach tag [$text tag names] {
	    set val [$text tag cget $tag -font]
	    if {[string is digit -strict [lindex $val 1]]} {
	        $text tag configure $tag -font "$searchfont \
		    [expr {max([lindex $val 1]+$offset,$min_fontsize)}]"
	    }
        }
    }
}

proc ::dialog_search::build_libdirs {textwidget} {
    set libdirs {} 
    foreach pathdir [concat $::sys_searchpath $::sys_staticpath] {
        if { ! [file isdirectory $pathdir]} {continue}
	# Fix the directory name, this ensures the directory name is in the 
	# native format for the platform and contains a final directory separator
	set dir [string trimright [file join [file normalize $pathdir] { }]]
	# find the libdirs
	foreach filename [glob -nocomplain -type d -path $dir "*"] {
	    # use [file tail $filename] to get the name of libdir
	    set dirname [file tail $filename]
	    set norm_filename [string trimright [file join [file normalize $filename] { }]]
	    if {[glob -nocomplain -type f -path $norm_filename "$dirname-meta.pd"] ne ""} {
		lappend libdirs [list "$norm_filename" "$dirname"]
	    }
	}
    }
    ::dialog_search::print_libdirs $textwidget $libdirs
}

proc ::dialog_search::print_libdirs {textwidget libdirs} {
    variable navbar {}
    $textwidget configure -state normal
    $textwidget delete 0.0 end
    lappend navbar [list "External libraries" "link libdirs navbar" {}]
    print_navbar $textwidget
    # now clear out the navbar and then add "externals (flag) to it..."
    foreach libdir [lsort $libdirs] {
	set description {}
	set author {}
	$textwidget insert end "[lindex $libdir 1]" "link clickable_dir dir_title"
	$textwidget insert end " "
        $textwidget image create end -image ::dialog_search::folder
	$textwidget tag add folder_icon "end -2indices" "end -1indices"
	$textwidget insert end "[lindex $libdir 0]" basedir
	$textwidget insert end "dummy" filename
	$textwidget insert end "1" is_libdir
	$textwidget insert end "\n"
	set file_contents [readfile [format %s%s [join $libdir ""] "-meta.pd"]]
	regexp -nocase -- "#X text \[0-9\]+ \[0-9\]+ description\[:\]? (.*?);.*" [join $file_contents] -> description
	if {$description ne {}} {
	    regsub -all { \\,} $description {,} description
	    $textwidget insert end "$description\n" description
	} else {
	    $textwidget insert end "no DESCRIPTION tag or values.\n" description
	}
	foreach tag {author license version} {
	    if {[regexp -nocase -- "#X text \[0-9\]+ \[0-9\]+ $tag (.*?);.*" [join $file_contents] -> values]} {
	        $textwidget insert end [format "%s: " $tag] metakey
		if {$values ne {}} {
	            regsub -all { \\,} $values {,} values 
	            $textwidget insert end "$values" metakey
	        } else {
	            $textwidget insert end "no AUTHOR tag or values." metakey
		}
	    $textwidget insert end "\n"
	    }
        }
    }
    $textwidget configure -state disabled
}

proc ::dialog_search::click_dir {textwidget xpos ypos} {
    set i [$textwidget index @$xpos,$ypos]
    set range [$textwidget tag nextrange basedir $i]
    set dir [eval $textwidget get $range]
    set range [$textwidget tag nextrange is_libdir $i]
    set is_libdir [eval $textwidget get $range]
    build_subdir $textwidget $dir $is_libdir
}

proc ::dialog_search::build_subdir {textwidget dir is_libdir} {
    variable navbar
    if {[lsearch -exact [join $navbar] $dir] == -1} {
    lappend navbar [list "$dir" "link clickable_dir navbar navbar_dir" "subdir"]
    } else {
        set newnav {}
	foreach {entry} $navbar {
	    lappend newnav $entry
	    if {[lindex $entry 0] eq $dir} {break}
	}
        set navbar $newnav
    }
    $textwidget configure -state normal
    $textwidget delete 0.0 end
    print_navbar $textwidget
    # get any subdirs first
    foreach subdir [lsort -dictionary [glob -nocomplain -type d -directory $dir "*"]] {
        # get name of subdir
        set subdirname [file tail $subdir]
	$textwidget insert end "$subdirname" "link clickable_dir dir_title"
        set norm_subdir [string trimright [file join [file normalize $subdir] { }]]
	$textwidget insert end "0" is_libdir
        $textwidget insert end " "
        $textwidget image create end -image ::dialog_search::folder
	$textwidget tag add folder_icon "end -2indices" "end -1indices"
        $textwidget insert end "$norm_subdir" basedir
	$textwidget insert end "\n"
    }
    foreach docfile [lsort -command directory_sort [find_doc_files [file normalize $dir] 0]] {
        # get name of file
	# if we're in a libdir, filter out pd patches that don't end in -help.pd
	if {[regexp {.*-help\.pd$} $docfile] ||
	    [string replace $docfile 0 [expr [string length $docfile] - 4]] ne ".pd" ||
	    !$is_libdir} {
            set docname [string replace $docfile 0 [string length [file normalize $dir]]]
	    ::dialog_search::printresult $docname $dir [readfile $docfile] $textwidget 0
	    }
    }
    $textwidget configure -state disabled
}

proc ::dialog_search::print_navbar {text} {
    variable navbar
    set separator -
    $text insert 1.0 "\n" navbar
    $text insert 1.0 "Home" "link intro navbar"
    if {[llength $navbar] == 0} {return}
    for {set i 0} {$i<[expr {[llength $navbar]-1}]} {incr i} {
        $text insert 1.end " $separator " navbar
        if {[lindex $navbar $i 2] eq "subdir"} {
	    $text insert 1.end [file tail [lindex $navbar $i 0]] \
		[lindex $navbar $i 1]
	    $text insert 1.end [lindex $navbar $i 0] basedir
	    $text insert 1.end dummy filename
	    $text insert 1.end "0" is_libdir
	} else {
	    $text insert 1.end [lindex $navbar $i 0] [lindex $navbar $i 1]
	}
    }
    if {[lindex $navbar end 2] eq "subdir"} {
        $text insert 1.end " $separator " navbar
	$text insert 1.end [file tail [lindex $navbar end 0]]
    } else {
	$text insert 1.end " $separator " navbar
        $text insert 1.end [lindex $navbar end 0]
    }
}

proc ::dialog_search::get_info {xpos ypos mytoplevel} {
    set textwidget "$mytoplevel.resultstext"
    set i [$textwidget index @$xpos,$ypos]
    set range [$textwidget tag nextrange filename $i]
    set filename [eval $textwidget get $range]
    set range [$textwidget tag nextrange basedir $i]
    set basedir [file normalize [eval $textwidget get $range]]
    set match 0
    set fulldir [file dirname [file join $basedir $filename]]
    set meta [format "%s-meta.pd" [file tail $fulldir]]
    if {[regexp {5.reference} $fulldir]} {
	tk_messageBox -message {Internal Object} \
	    -detail {This help patch is for an internal Pd class} \
	    -parent $mytoplevel -title Search
	set match 1
    } else {
	# check for a readme file (use libname-meta.pd as a last resort)
	foreach docname [list Readme.txt README.txt readme.txt README $meta] {
	    if {[file exists [file join $fulldir $docname]]} {
		menu_doc_open $fulldir $docname
		set match 1
		break
	    }
	}
    }
    if {!$match} {
	tk_messageBox -message {Sorry, can't find a README file\
	    for this object's library.} -title Search
    }    	
}

proc ::dialog_search::create_dialog {mytoplevel} {
    variable searchfont
    variable selected_file
    variable genres
    variable count
    foreach genre $genres {
	lappend count 0
    }
    toplevel $mytoplevel -class DialogWindow
    wm title $mytoplevel [_ "Search"]
    wm geometry $mytoplevel 670x500+0+30
    # style tweak: get rid of arrow so the combobox looks like a simple entry widget
    ttk::style configure Entry.TCombobox -arrowsize 0
    ttk::style configure Genre.TCombobox
    ttk::style configure Search.TButton -font menufont
    ttk::style configure Search.TCheckbutton -font menufont
    # widgets
    # for some reason ttk widgets didn't inherit the menufont, and this causes tiny
    # fonts on Windows-- so let's hack!
    foreach widget {f.genrebox advancedlabel } {
        option add *[string trim "$mytoplevel.$widget" .]*font menufont
    }
    foreach combobox {searchtextentry f.genrebox} {
	option add *[string trim "$mytoplevel.$combobox" .]*Listbox.font menufont
    }
    ttk::combobox $mytoplevel.searchtextentry -textvar ::dialog_search::searchtext \
	-font "$searchfont 12" -style "Entry.TCombobox" -cursor "xterm"
    ttk::button $mytoplevel.searchbutton -text [_ "Search"] -takefocus 0 \
	-command ::dialog_search::search -style Search.TButton
    ttk::frame $mytoplevel.f -padding 2
    ttk::combobox $mytoplevel.f.genrebox -values $genres -state readonly -style "Genre.TCombobox"
    $mytoplevel.f.genrebox current 0
    ttk::checkbutton $mytoplevel.f.matchall_check -text [_ "Match all terms"] \
	-variable ::dialog_search::matchall -style Search.TCheckbutton
    ttk::checkbutton $mytoplevel.f.matchwords_check -text [_ "Match whole words"] \
	-variable ::dialog_search::matchwords -style Search.TCheckbutton
    ttk::label $mytoplevel.advancedlabel -text [_ "Advanced"] -foreground "#0000ff" \
	-anchor center
    text $mytoplevel.resultstext -yscrollcommand "$mytoplevel.yscrollbar set" \
        -bg white -highlightcolor blue -height 30 -wrap word -state disabled \
	-padx 8 -pady 4 -spacing3 2
    ttk::scrollbar $mytoplevel.yscrollbar -command "$mytoplevel.resultstext yview" \
        -takefocus 0
    ttk::label $mytoplevel.statusbar -text "Pure Data Search" -justify left \
        -padding {4 4 4 4}

    grid $mytoplevel.f.genrebox $mytoplevel.f.matchall_check \
	$mytoplevel.f.matchwords_check -padx 3
    grid $mytoplevel.searchtextentry -column 0 -columnspan 3 -row 0 -padx 3 -pady 2 -sticky ew
    grid $mytoplevel.searchbutton -column 3 -columnspan 1 -row 0 -padx 3 -sticky ew
    grid $mytoplevel.f -column 0 -columnspan 3 -row 1 -sticky we
    grid $mytoplevel.advancedlabel -column 3 -columnspan 1 -row 1 -sticky ew 
    grid $mytoplevel.resultstext -column 0 -columnspan 4 -row 2 -sticky nsew
    grid $mytoplevel.yscrollbar -column 4 -row 2 -sticky nsew
    grid $mytoplevel.statusbar -column 0 -columnspan 4 -row 3 -sticky nsew
    grid columnconfigure $mytoplevel 0 -weight 1
    grid columnconfigure $mytoplevel 4 -weight 0
    grid rowconfigure    $mytoplevel 2 -weight 1

    # tags
    $mytoplevel.resultstext tag configure hide -elide on
    $mytoplevel.resultstext tag configure is_libdir -elide on
    $mytoplevel.resultstext tag configure title -foreground "#0000ff" -underline on \
	-font "$searchfont 12" -spacing1 6
    $mytoplevel.resultstext tag configure dir_title -font "$searchfont 12" \
	-underline on -spacing1 6
    $mytoplevel.resultstext tag configure filename -elide on
    $mytoplevel.resultstext tag configure metakey -font "$searchfont 10"
    $mytoplevel.resultstext tag configure metavalue_h -elide on
    $mytoplevel.resultstext tag configure basedir -elide on
    $mytoplevel.resultstext tag configure description -font "$searchfont 12"
    $mytoplevel.resultstext tag configure homepage_title -font "$searchfont 12" \
	-underline on -spacing1 10 -spacing3 5
    $mytoplevel.resultstext tag configure homepage_description -font "$searchfont 12" \
	-spacing3 7
    $mytoplevel.resultstext tag configure navbar -font "$searchfont 10" \
	-spacing1 8 -spacing3 5
    $mytoplevel.resultstext tag configure intro_libdirs -font "$searchfont 12"
    # create a bunch of link bindings so that you get <Enter> and <Leave>
    # events when two links are right next to each other
    $mytoplevel.resultstext tag configure link -foreground "#0000ff"
    $mytoplevel.resultstext tag bind link <Enter> "$mytoplevel.resultstext configure \
        -cursor hand2"
    $mytoplevel.resultstext tag bind link <Leave> "$mytoplevel.resultstext configure \
        -cursor xterm; $mytoplevel.statusbar configure -text \"\""
    $mytoplevel.resultstext tag bind intro <Button-1> "::dialog_search::intro \
	$mytoplevel.resultstext"
    $mytoplevel.resultstext tag bind intro <Enter> "$mytoplevel.statusbar \
	configure -text \"Go back to the main help page\""
    $mytoplevel.resultstext tag bind intro <Leave> "$mytoplevel.statusbar \
	configure -text \"\""
    $mytoplevel.resultstext tag bind libdirs <Button-1> "::dialog_search::build_libdirs \
	$mytoplevel.resultstext"
    $mytoplevel.resultstext tag bind libdirs <Enter> "$mytoplevel.statusbar configure \
	-text \"Browse all external libraries that have the libdir format\""
    $mytoplevel.resultstext tag bind libdirs <Leave> "$mytoplevel.statusbar configure \
	-text \"\""
    # hack to force new <Enter> events for tags and links next to each other
    for {set i 0} {$i<30} {incr i} {
	$mytoplevel.resultstext tag bind "metavalue$i" <Button-1> \
	    "::dialog_search::grab_metavalue %x %y $mytoplevel 1"
        $mytoplevel.resultstext tag bind "metavalue$i" <Enter> \
	    "::dialog_search::grab_metavalue %x %y $mytoplevel 0"
	$mytoplevel.resultstext tag bind "intro_link$i" <Enter> \
	    "::dialog_search::open_file %x %y $mytoplevel dir 0"
	$mytoplevel.resultstext tag bind "intro_link$i" <Leave> \
	    "$mytoplevel.statusbar configure -text \"\""
	$mytoplevel.resultstext tag configure "metavalue$i" -font \
	    "$searchfont 12"
	$mytoplevel.resultstext tag configure "intro_link$i" -font \
	    "$searchfont 12"
    }
    # this next tag configure comes after the metavalue stuff above so
    # that it has a higher priority (these are the keywords in the search
    # results)
    $mytoplevel.resultstext tag configure keywords -font "$searchfont 10"
    $mytoplevel.resultstext tag configure homepage_file -font "$searchfont 12"
    $mytoplevel.resultstext tag bind homepage_file <Button-1> "::dialog_search::open_file \
	%x %y $mytoplevel file 1"
    $mytoplevel.resultstext tag bind homepage_file <Enter> "::dialog_search::open_file \
	%x %y $mytoplevel file 0"
    $mytoplevel.resultstext tag bind homepage_file <Leave> "$mytoplevel.statusbar configure \
	-text \"\""
    $mytoplevel.resultstext tag bind title <Button-1> "::dialog_search::open_file %x %y \
	$mytoplevel file 1"
    $mytoplevel.resultstext tag bind title <Enter> "::dialog_search::open_file %x %y \
        $mytoplevel file 0"
    $mytoplevel.resultstext tag bind dir_title <Enter> "::dialog_search::open_file %x %y \
	$mytoplevel dir 0"
    $mytoplevel.resultstext tag bind dir_title <Leave> "$mytoplevel.resultstext configure \
	-cursor xterm; $mytoplevel.statusbar configure -text \"\""
    $mytoplevel.resultstext tag bind help_icon <Button-1> "::dialog_search::get_info %x %y \
	$mytoplevel"
    $mytoplevel.resultstext tag bind help_icon <Enter> "$mytoplevel.resultstext configure \
	-cursor hand2; $mytoplevel.statusbar configure -text \"Get info on this object's\
	libdir\""
    $mytoplevel.resultstext tag bind help_icon <Leave> "$mytoplevel.resultstext configure \
	-cursor xterm; $mytoplevel.statusbar configure -text \"\""
    $mytoplevel.resultstext tag bind folder_icon <Button-1> "::dialog_search::open_file %x %y \
	$mytoplevel dir_in_fm 1"
    $mytoplevel.resultstext tag bind folder_icon <Enter> "::dialog_search::open_file %x %y \
	$mytoplevel dir_in_fm 0"
    $mytoplevel.resultstext tag bind folder_icon <Leave> "$mytoplevel.resultstext configure \
	-cursor xterm; $mytoplevel.statusbar configure -text \"\""
    $mytoplevel.resultstext tag bind clickable_dir <Button-1> "::dialog_search::click_dir \
	$mytoplevel.resultstext %x %y"
    # another workaround: we can't just do a mouseover statusbar update with clickable_dir 
    # since it wouldn't register an <Enter> event when moving the mouse from one dir to an 
    # adjacent dir. So we have the intro_link$i hack above PLUS a separate binding for navbar 
    # links (which are not adjacent)
    $mytoplevel.resultstext tag bind navbar_dir <Enter> "::dialog_search::open_file %x %y \
	$mytoplevel dir 0"
    $mytoplevel.resultstext tag bind navbar_dir <Leave> "$mytoplevel.statusbar configure \
	-text \"\""

    # search window widget bindings
    bind $mytoplevel <$::modifier-equal> "::dialog_search::font_size $mytoplevel.resultstext 1"
    bind $mytoplevel <$::modifier-plus> "::dialog_search::font_size $mytoplevel.resultstext 1"
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
	-cursor hand2; $mytoplevel.statusbar configure -text \"Advanced search options\""
    bind $mytoplevel.advancedlabel <Leave> "$mytoplevel.advancedlabel configure \
	-cursor xterm; $mytoplevel.statusbar configure -text \"\""
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

# Folder icon "folder16"
# from kde klassic icons (license says GPL/LGPL)

image create photo ::dialog_search::folder -data {
   R0lGODlhEAAQAIMAAPwCBNSeBJxmBPz+nMzOZPz+zPzSBPz2nPzqnAAAAAAA
   AAAAAAAAAAAAAAAAAAAAACH5BAEAAAAALAAAAAAQABAAAARFEMhJ6wwYC3uH
   98FmBURpElkmBUXrvsVgbOxw3F8+A+zt/7ddDwgUFohFWgGB9BmZzcMTASUK
   DdisNisSeL9gMGdMJvsjACH+aENyZWF0ZWQgYnkgQk1QVG9HSUYgUHJvIHZl
   cnNpb24gMi41DQqpIERldmVsQ29yIDE5OTcsMTk5OC4gQWxsIHJpZ2h0cyBy
   ZXNlcnZlZC4NCmh0dHA6Ly93d3cuZGV2ZWxjb3IuY29tADs=
}

# Info icon "acthelp16"
# from kde slick icons (license says GPL/LGPL)

image create photo ::dialog_search::help -data {
   R0lGODlhEAAQAIYAAPwCBCSK7ASO9Bye9GTG/ITO/JzW/HTC/FS6/Bya/ARe
   3FS+/Jzi/LTm/Mzu/LTi/KTa/HzC/GSy/AR+/AQudGy+/MTq/Nzy/OT2/Lzm
   /KTi/ITK/Hy+/Fym/CSG/ARSzAQmZCyW9OTy/Pz+/Oz2/Dye/BR2/ARm/AQ2
   pPT+/Oz6/NTy/ESe/BR+/ARO3AQ+tAQWVAyW/HzK/NT2/DyW/ARa9ARC5AQ2
   rAQmhAyi/HzO/Jze/DSK/ARO7ARCzASa7HzS/HzG/ITG/CR2/AQ+zAQupAR6
   tDyu/Eyy/AQ+1AQaZASG/Fy2/CSe/ARC3AQytAQqlAQONBSO/Hyi3DRmzARm
   rARq5ARS3ARS5ARGzAQ2tAQmjARCpAQ6rAQ6tAQebAQaXAQqfAQqhAAAAAAA
   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACH5BAEAAAAA
   LAAAAAAQABAAAAfFgACCgwABAoSIggMEBQYHCAkKiQALDA0ODxAREhMUiBUW
   FxgZGhscHR4fIIMhDyIjJA4jIyUhJicogwQPKSojK7MsLScuLzAAMTIzDiIr
   DgYcNCY1Njc4ADk6Fjs7DbMjPCc9PtYAPwgWBEBBQrND1ERF10ZHMghHSCyz
   4klFRUqCljBp0sSDh1k9nDx5AiXKIAFSJkz5NoLKExzHBlWxcuIKlixJtDzZ
   osQhIRAUuHTxgqLIli+TBIEJIwYHmIwxcwryEwgAIf5oQ3JlYXRlZCBieSBC
   TVBUb0dJRiBQcm8gdmVyc2lvbiAyLjUNCqkgRGV2ZWxDb3IgMTk5NywxOTk4
   LiBBbGwgcmlnaHRzIHJlc2VydmVkLg0KaHR0cDovL3d3dy5kZXZlbGNvci5j
   b20AOw==
}
