proc getfiles {dirlist} {
  set filelist {}
  foreach dir $dirlist {
  		lappend filelist [glob -nocomplain -type {f r} -path [string trimright [file join $dir { }]]  "*.{pd,pat,mxb,mxt,help,txt,htm,html,pdf}"]
  		}
  return $filelist
}

proc getdirs {} {
  set dirlist {}
  foreach dir [glob -nocomplain -type d -path [string trimright [file join . { }]] *] {
    lappend dirlist $dir
  }
  return $dirlist
}

proc readfiles {filenames} {
  foreach filename [join $filenames] {
    readfile $filename
  }
}

proc readfile {filename} {
  set fp [open $filename]
  fconfigure $fp -buffering full
  set file_contents [read $fp]
  close $fp
}

proc runtest {} {
puts "Getting dirs takes [time {set dirs [getdirs]}]"
puts "Getting files takes [time {set files [getfiles $dirs]}]"
puts "Reading files takes [time {readfiles $files}]"
puts "Done."
}

runtest
