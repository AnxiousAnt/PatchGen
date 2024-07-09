proc LoadBLT { version dir } {

    set prefix ""
    set suffix [info sharedlibextension]
    regsub {\.} $version {} version_no_dots

    # Determine whether to load the full BLT library or
    # the "lite" tcl-only version.
    
    if { [info commands tk] == "tk" } {
        set name ${prefix}BLT${version_no_dots}${suffix}
    } else {
        set name ${prefix}BLTlite${version_no_dots}${suffix}
    }
    
    global tcl_platform
	set library [file join $dir $name]
    load $library BLT
}

set version "2.4"

package ifneeded BLT $version [list LoadBLT $version $dir]
# End of package index file



package ifneeded snack 2.2 "[list load [file join $dir libsnack.dll]];[list source [file join $dir snack.tcl]]"

package ifneeded sound 2.2 [list load [file join $dir libsound.dll]]

package ifneeded snacksphere 1.2 [list load [file join $dir libsnacksphere.dll]]

package ifneeded snackogg 1.3 [list load [file join $dir libsnackogg.dll]]




package ifneeded Tkzinc 3.3.2 [list load [file join $dir Tkzinc332.dll]]
package ifneeded zincText 1.0 [list source [file join $dir zincText.tcl]]
package ifneeded zincLogo 1.0 [list source [file join $dir zincLogo.tcl]]
package ifneeded zincGraphics 1.0 [list source [file join $dir zincGraphics.tcl]]

if {[catch {package require Tcl 8.2}]} return
package ifneeded Tktable 2.7 "package require Tk 8.2; [list load [file join $dir Tktable.dll] Tktable]"

if {[catch {package require Tcl 8.4}]} return
set script ""
if {![info exists ::env(TREECTRL_LIBRARY)]
    && [file exists [file join $dir treectrl.tcl]]} {
    append script "set ::treectrl_library \"$dir\"\n"
}
append tree "load \"[file join $dir treectrl21.dll]\" treectrl"
package ifneeded treectrl 2.1 $tree

if {[catch {package require Tcl 8.4}]} return
set script ""

append script "load \"[file join $dir libtkdnd10.dll]\" tkdnd"
package ifneeded tkdnd 1.0 $script

if {[catch {package require Tcl 8.4}]} return
set script ""

append script "load \"[file join $dir tkpathgdi01.dll]\" tkpath"
package ifneeded tkpath 0.1 $script



if {![package vsatisfies [package provide Tcl] 8.4]} {return}
if {[package vsatisfies [package provide Tcl] 8.5] 
    || [package vsatisfies [info patchlevel] 8.4.6]} {
    package ifneeded tile 0.7.2 \
        "namespace eval tile {variable library \"$dir\"};\
         load \"[file join $dir tile072t.dll]\""
}

if {[catch {package require Tcl 8.2}]} return
package ifneeded Tktable 2.7 "package require Tk 8.2; [list load [file join $dir Tktable.dll] Tktable]"

if {![package vsatisfies [package provide Tcl] 8.4]} {return}
if {[package vsatisfies [package provide Tcl] 8.5] 
    || [package vsatisfies [info patchlevel] 8.4.6]} {
    package ifneeded tile 0.7.2 \
        "namespace eval tile {variable library \"$dir\"};\
         load \"[file join $dir tile072t.dll]\""



