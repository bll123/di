#!/bin/sh

echo "Enter version: \c"
read ver

rm -rf di-${ver} > /dev/null 2>&1
mkdir di-${ver}
mkdir di-${ver}/mkconfig
chmod 755 di-${ver}
chmod 755 di-${ver}/mkconfig

for f in $@; do
  case $f in 
    mkconfig*)
      cp -r $f di-${ver}/mkconfig
      ;;
    *)
      cp -r $f di-${ver}
      ;;
  esac
done
chmod -R a+r di-${ver}/*

tar cf - ./di-${ver} |
gzip -9 > di-${ver}.tar.gz

rm -rf di-${ver} > /dev/null 2>&1

exit 0
