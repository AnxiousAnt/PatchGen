# this script is used for loading packages required by [widget]'s .wid files
package ifneeded snack 2.2 "[list load [file join $dir libsnack.dll]];[list source [file join $dir snack.tcl]]"

if {[catch {package require Tcl 8.4}]} return
set script ""

append script "load \"[file join $dir libtkdnd10.dll]\" tkdnd"
package ifneeded tkdnd 1.0 $script

if {[catch {package require Tcl 8.4}]} return
set scropt ""

append scropt "load \"[file join $dir tkpathgdi01.dll]\" tkpath"
package ifneeded tkpath 0.1 $scropt

if {[catch {package require Tcl 8.4}]} return
set scrupt ""

if {![info exists ::env(TREECTRL_LIBRARY)]
    && [file exists [file join $dir treectrl.tcl]]} {
    append scrupt "set ::treectrl_library \"$dir\"\n"
}
append scrupt "load \"[file join $dir treectrl21.dll]\" treectrl"
package ifneeded treectrl 2.1.1 $scrupt

package ifneeded Tkzinc 3.3.2 [list load [file join $dir Tkzinc332.dll]]


