package require Tcl 8.5
package require Tk
package require pdwindow 0.1

namespace eval ::modelabel:: { }

proc ::modelabel::show_hide {mytoplevel} {
    if {$mytoplevel eq ".pdwindow"} {return}
    if { ! [winfo exists $mytoplevel] } {return}
    set tkcanvas [tkcanvas_name $mytoplevel]
    set labelId $mytoplevel.modelabel

    if { ! [winfo exists $labelId]} {
        $tkcanvas create text 10 10 -tags $labelId -fill "#888888" -width 100 -anchor w
    }

    if {$::editmode($mytoplevel)} {
         $tkcanvas itemconfig $labelId -text "EDIT"
    } else {
         $tkcanvas itemconfig $labelId -text "ACTION"
    }
}

bind PatchWindow <<EditMode>> {+::modelabel::show_hide %W}
bind PatchWindow <<Loaded>> {+::modelabel::show_hide %W}

pdtk_post "Loaded Show Mode Plugin\n"
