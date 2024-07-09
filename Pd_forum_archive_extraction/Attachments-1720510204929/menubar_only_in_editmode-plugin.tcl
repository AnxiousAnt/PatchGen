# this GUI plugin removes the menubars from any patch window that is
# not in Edit Mode.  Also, if a patch is switched to Run Mode, the
# menubar will be removed.

proc setmenu_for_editmode {mytoplevel} {
    if {$::editmode($mytoplevel)} {
        $mytoplevel configure -menu $::patch_menubar
    } else {
        $mytoplevel configure -menu ""
    }
}

# on Mac OS X, no windows have menubars, so no need to remove for 'aqua'
if {$::windowingsystem ne "aqua"} {
    bind PatchWindow <FocusIn> {+setmenu_for_editmode %W}
    bind PatchWindow <<EditMode>> {+setmenu_for_editmode %W}
}

pdtk_post "Finished loading menubar_only_in_editmode-plugin.tcl"
