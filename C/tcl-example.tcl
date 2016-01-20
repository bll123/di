#!/usr/bin/tclsh
#
# example tcl script
#
# To build the sharedlibrary:
#   make tcl-sh
#
# the load wants the full path, not relative...
set ext [info sharedlibextension]
set lfn [file normalize [file join [file dirname [info script]] diskspace$ext]]
load $lfn

puts "== Default format, all filesystems"
puts [diskspace]
puts "== Default format, just /"
puts [diskspace /]
puts "== Change the format, just /"
puts [diskspace -f SMpT /]
puts "== All filesystems, specified format"
puts [diskspace -f SMpT]
