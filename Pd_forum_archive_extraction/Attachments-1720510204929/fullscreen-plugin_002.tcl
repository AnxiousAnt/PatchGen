
# META NAME Fullscreen for F11
# META DESCRIPTION Lets you switch to and back from fullscreen with F11
# META AUTHOR <András Murányi> muranyia@gmail.com

package require Tcl 8.5
package require Tk
package require pdwindow 0.1
package require pd_menus 0.1

variable fullscreen_windows {}

proc go_fullscreen {window} {
	global fullscreen_windows
#	wm attributes $window -topmost yes   ;# stays on top
	wm attributes $window -fullscreen 1
	if {$::windowingsystem ne "aqua"} {
    $window configure -menu ""
 	} 
	lappend fullscreen_windows $window
	update idletasks
}

proc withdraw_fullscreen {window} {
	global fullscreen_windows
#	if { [wm attributes $window -topmost] eq "yes" } {
#		wm attributes $window -topmost no
#	}
	wm attributes $window -fullscreen 0
	if {$::windowingsystem ne "aqua"} {
    $window configure -menu $::patch_menubar
 	} 
	set idx [lsearch -exact $fullscreen_windows $window]
	set fullscreen_windows [lreplace $fullscreen_windows $idx $idx]
	update idletasks
}

proc toggle_fullscreen {window} {
	global fullscreen_windows
	set window [winfo toplevel $window]
	if { [winfo class $window] eq "PatchWindow"} {
		if { [lsearch $fullscreen_windows $window] == -1 } {
			go_fullscreen $window
		} else {
			withdraw_fullscreen $window
		}
	}
}

bind all <F11> {toggle_fullscreen %W}