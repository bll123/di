#!/bin/sh

echo "Enter version: \c"
read ver
rm -rf di-${ver} > /dev/null 2>&1
mkdir di-${ver}
chmod 755 di-${ver}
cp -r $* di-${ver}
chmod -R a+r di-${ver}/*
tar cf - ./di-${ver} |
gzip -9 > di-${ver}.tar.gz
rm -rf di-${ver} > /dev/null 2>&1
exit 0
