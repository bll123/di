#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

. ./VERSION.txt
ver=$DI_VERSION

PKG=di
cwd=$(pwd)
dir="${PKG}-${ver}"
rm -rf $dir > /dev/null 2>&1
mkdir $dir
mkdir $dir/utils
chmod 755 $dir

cp -pf *.c *.h *.in $dir
cp -pf VERSION.txt CMakeLists.txt LICENSE.txt Makefile $dir
cp -pf README.txt $dir
cp -pf utils/chkcmake.sh utils/chklibnm.sh utils/instpo.sh $dir/utils
for d in mkconfig mkc_config man po; do
  cp -p -rf ${d} $dir
done

chmod -R a+r $dir
rm -f *.tar.gz
tar -c -f - $dir |
    gzip -9 > $dir.tar.gz

rm -rf $dir > /dev/null 2>&1

exit 0
