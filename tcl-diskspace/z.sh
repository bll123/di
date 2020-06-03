#!/bin/bash

ver=4.47.3
echo "version $ver"

rm -f *~ > /dev/null 2>&1

top=diskspace-${ver}
zip=tcl-diskspace-${ver}
test -d $top && rm -rf $top
cp -r binaries $top
cp pkgIndex.tcl README.txt $top
test -f ${top}.zip && rm -f ${top}.zip
zip -rq ${zip}.zip ${top}
rm -rf $top
