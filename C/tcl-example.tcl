#!/usr/bin/tclsh
#
# example tcl script
#
# To build the sharedlibrary:
#   make -e tcl-sh
#
# the load wants the full path, not relative...
set ext [info sharedlibextension]
set lfn [file normalize [file join [file dirname [info script]] diskspace$ext]]
load $lfn

puts "== Basic information only"
set d [diskspace -f {}]
dict for {mount dline} $d {
  puts "$mount: $dline"
}
puts "== Basic information and some display data"
set d [diskspace -f buvp]
dict for {mount dline} $d {
  puts "$mount: $dline"
}
# dictionary is nested with the mount point as the key.
puts "/ total space: [dict get $d / total]"
