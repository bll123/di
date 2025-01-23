#!/bin/sh

ver=`grep "^DI_VERSION" Makefile | sed -e 's/.*= *//'`

PKG=di
cwd=`pwd`
dir="${PKG}-${ver}"
rm -rf $dir > /dev/null 2>&1
mkdir $dir
chmod 755 $dir

make distclean

cp -f *.c *.h *.in $dir
cp -f CMakeLists.txt LICENSE.txt Makefile $dir
cp -f README.txt README-new.txt $dir
for d in mkconfig mkc_config man po; do
  cp -rf ${d} $dir
done

chmod -R a+r $dir
tar cf - $dir |
    gzip -9 > $dir.tar.gz

rm -rf $dir > /dev/null 2>&1

exit 0
