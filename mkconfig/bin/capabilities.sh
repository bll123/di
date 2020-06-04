#!/bin/sh
#
# Copyright 2020 Brad Lanam Pleasant Hill CA USA
#

if [ "${_MKCONFIG_DIR}" = "" ]; then
  echo "Usage: _MKCONFIG_DIR=<path> <shell> bin/capabilities.sh"
  exit 1
fi

unset CDPATH
# this is a workaround for ksh93 on solaris
if [ "$1" = "-d" ]; then
  cd $2
  shift
  shift
fi
. ${_MKCONFIG_DIR}/bin/shellfuncs.sh
doshelltest $0 $@

puts "$shell $shvers"
puts "printf: $shhasprintf"
puts "append: $shhasappend"
puts "math: $shhasmath"
puts "typeset -u: $shhasupper"
puts "typeset -l: $shhaslower"
