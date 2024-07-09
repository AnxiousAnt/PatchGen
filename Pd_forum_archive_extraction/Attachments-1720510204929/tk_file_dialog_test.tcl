#!/usr/local/bin/tclsh

# I am not familiar with tcl/tk, so bear with me ...

# put together from http://www.groupsrv.com/computers/about266840-0-asc-15.html

package require Tk

# apparently, you need to somehow open or load the filedialog once before
# changing the environment variables will work
catch {tk_getOpenFile -with-invalid-argument} 

# change the environment variables
namespace eval ::tk::dialog::file {
variable showHiddenBtn 1
variable showHiddenVar 0
}

# open dialog
set filename [tk_getOpenFile -initialdir ~/ -title "open dialog test"]

puts $filename

exit

