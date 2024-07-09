
# META NAME Fullscreen for F11
# META DESCRIPTION Lets you switch to and back from fullscreen with F11
# META AUTHOR <András Murányi> muranyia@gmail.com

package require Tcl 8.5
package require Tk
package require pdwindow 0.1

variable fullscreen 0

proc toggle_fullscreen {window} {
	global fullscreen
	set goal [expr ! $fullscreen]
	set w [winfo toplevel $window]
	if { [winfo class $w] ne "PdWindow"} {
		if { $goal eq 1 } {
#			pack forget $w.menubar
		} else {
#			pack $w.menubar
		}
		wm attributes $w -fullscreen $goal
#		wm attributes $w -fullscreen $goal -topmost $goal
		set fullscreen $goal
	}
}

bind all <Key-F11> "+toggle_fullscreen %W"

