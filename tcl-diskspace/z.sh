#!/bin/bash

ver=$(grep DI_VERSION ../C/version.h | sed -e 's/"$//' -e 's/.*"//')
echo "version $ver"

rm -f *~ > /dev/null 2>&1

top=diskspace-${ver}
zip=tcl-diskspace-${ver}
test -d $top && rm -rf $top
cp -r binaries $top
cp pkgIndex.tcl README.txt $top
test -f ${zip}.zip && rm -f ${zip}.zip
zip -rq ${zip}.zip ${top}
rm -rf $top
