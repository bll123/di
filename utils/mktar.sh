#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

beta=""
if [[ $1 == beta ]]; then
  beta=-beta
fi

. ./VERSION.txt
ver=$DI_VERSION

PKG=di
cwd=$(pwd)
dir="${PKG}-${ver}${beta}"
rm -rf $dir > /dev/null 2>&1
mkdir $dir
mkdir $dir/utils
mkdir $dir/tests
mkdir $dir/examples
chmod 755 $dir

cp -pf *.c *.h *.in \
    VERSION.txt CMakeLists.txt LICENSE.txt Makefile \
    README.md \
    $dir
cp -pf utils/chkcmake.sh utils/instlibdi.sh \
    utils/instman.sh utils/instpo.sh utils/instpc.sh \
    $dir/utils
cp -pf tests/localtest.sh \
    $dir/tests
cp -prf examples $dir
for d in mkconfig mkc_config man po; do
  cp -p -rf ${d} $dir
done

chmod -R a+r $dir
rm -f *.tar.gz
tar -c -f - $dir |
    gzip -9 > $dir.tar.gz

rm -rf $dir > /dev/null 2>&1

exit 0
