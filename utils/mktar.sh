#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

ver=$(grep "^DI_VERSION" Makefile | sed -e 's/.*= *//')

PKG=di
cwd=$(pwd)
dir="${PKG}-${ver}"
rm -rf $dir > /dev/null 2>&1
mkdir $dir
mkdir $dir/utils
chmod 755 $dir

cp -pf *.c *.h *.in $dir
cp -pf CMakeLists.txt LICENSE.txt Makefile $dir
cp -pf README.txt README-new.txt $dir
cp -pf utils/chkcmake.sh utils/instpo.sh $dir/utils
for d in mkconfig mkc_config man po; do
  cp -p -rf ${d} $dir
done

chmod -R a+r $dir
rm -f *.tar.gz
tar cf - $dir |
    gzip -9 > $dir.tar.gz

rm -rf $dir > /dev/null 2>&1

exit 0
