
# for more info: http://www.tcl.tk/man/tcl8.5/TkCmd/ttk_notebook.htm

package require Tcl 8.5
package require Ttk

namespace import ::pdwindow::pdtk_post



pack [ttk::notebook .pdwindow.tabs]
pack .pdwindow.tabs -side right -fill both -expand 1

.pdwindow.tabs add [ttk::frame .pdwindow.tabs.f1] -text "Console"

pack .pdwindow.scroll -in .pdwindow.tabs.f1 -side right -fill y
pack .pdwindow.text -in .pdwindow.tabs.f1 -side right -fill both -expand 1
raise .pdwindow.scroll
raise .pdwindow.text
#.pdwindow.text configure -state disabled

.pdwindow.tabs add [frame .pdwindow.tabs.f2] -text "Second tab"

.pdwindow.tabs select .pdwindow.tabs.f1

ttk::notebook::enableTraversal .pdwindow.tabs





