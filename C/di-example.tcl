#!/usr/bin/tclsh
#
# example tcl script
#
# the load wants the full path, not relative...
#
set lfn [file normalize [file join [file dirname [info script]] diskspace.so]]
load $lfn
puts [diskspace /]
