package require tkdnd

namespace eval ::dnd_object_create {
    variable x 0
    variable y 0
}

#------------------------------------------------------------------------------#
# create an object using the dropped filename

bind PatchWindow <<Loaded>> {+::dnd_object_create::bind_to_canvas %W}

    ::pdwindow::post "drop on canvas loaded\n"

proc ::dnd_object_create::bind_to_canvas {mytoplevel} {
    ::tkdnd::drop_target register $mytoplevel DND_Files
    bind $mytoplevel <<DropPosition>> {::dnd_object_create::setxy %X %Y}
    bind $mytoplevel <<Drop:DND_Files>> {::dnd_object_create::make_object %W %D}
}


proc ::dnd_object_create::setxy {newx newy} {
    variable x $newx
    variable y $newy
    return "copy"
}


proc ::dnd_object_create::make_object {mytoplevel files} {
    foreach file $files {
	set ext  [file extension $file]
	set obj  [file rootname [file tail $file]]
	set dir  [file dirname $file]
    if {$ext == ".pd"} {
        foreach pathdir [concat $::sys_searchpath $::sys_staticpath] {
            if {$pathdir == $dir} {
                ::dnd_object_create::test $mytoplevel $obj
                ::pdwindow::post "$pathdir dropped on $::focused_window\n"
                break
                }
	    }
        }
    }
    return "link"
}

proc ::dnd_object_create::test {w obj} {
    variable x
    variable y
    pdsend "$w obj $x $y $obj"
    return "dropped"
}

