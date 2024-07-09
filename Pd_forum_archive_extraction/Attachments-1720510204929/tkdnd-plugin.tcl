package require tkdnd

dnd bindtarget .pdwindow text/uri-list <Drop> {
    foreach file %D {open_file $file}
}

proc setup_dndbind {mytoplevel} {
    ::pdwindow::error "binding to $mytoplevel"
    dnd bindtarget $mytoplevel text/uri-list <Drop> "pdtk_canvas_makeobjs $mytoplevel %D %x %y"
}
bind PatchWindow <<Loaded>> {+setup_dndbind %W}

proc pdtk_canvas_makeobjs {mytoplevel files x y} {
    set c 0
    for {set n 0} {$n < [llength $files]} {incr n} {
        if {[regexp {.*/(.+).pd$} [lindex $files $n] file obj] == 1} {
            ::pdwindow::error " do it $file $obj $x $y $c"
            pdsend "$mytoplevel obj $x [expr $y + ($c * 30)] $obj"
            incr c
        }
    } 
}
