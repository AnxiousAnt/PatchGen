# META NAME replace open dialog with pd_gtk_open gtk application
# META DESCRIPTION Gives you a gtk-looking open. Requires the pd_gtk_open binary
# META AUTHOR <Lorenzo Sutton> lsutton@libero.it
package require Tcl 8.5
package require Tk
package require pdwindow 0.1
package require pd_menus 0.1
package require pd_menucommands 0.1

#rename ::pd_menucommands::menu_open ::pd_menucommands::menu_open_old

set ::pd_gtk_open_path $::sys_guidir/pd_gtk_open

proc ::pd_menucommands::menu_open {} {
    set gtk_ok 0
    if {[file exists $::pd_gtk_open_path]} {
        set files ""
        if {! [catch {exec $::pd_gtk_open_path $::fileopendir} fileList]} {
            #puts "Using pd_gtk_open"
            set files [split $fileList "|"]
            set gtk_ok 1
        }
    # if the executable pd_gtk_open is not found fall-back to tk
    } else {
        if { ! [file isdirectory $::fileopendir]} {set ::fileopendir $::env(HOME)}
        set files [tk_getOpenFile -defaultextension .pd \
	                       -multiple true \
	                       -filetypes $::filetypes \
	                       -initialdir $::fileopendir]
    }
    if {$files ne ""} {
        foreach filename $files { 
            open_file $filename
        }
        set ::fileopendir [file dirname $filename]
    }
}

# find the binary file in Pd's path
foreach pathdir [concat $::sys_searchpath $::sys_staticpath] {
    set dir [file normalize $pathdir]
    set testfile [file join $dir pd_gtk_open]
    if { [file exists $testfile]} {
        if {[file executable $testfile]} {
            set ::pd_gtk_open_path $testfile
        } else {
            pdtk_post "$testfile exists, but is not executable.\n"
            pdtk_post "To fix, run: \tsudo chmod +x $testfile\n"
        }
    }
}
