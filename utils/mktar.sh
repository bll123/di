#!/bin/sh

ver=`grep "^DI_VERSION" Makefile | sed -e 's/.*= *//'`

PKG=di
cwd=`pwd`
dir="${PKG}-${ver}"
rm -rf $dir > /dev/null 2>&1
mkdir $dir
chmod 755 $dir

make distclean

cp -pf *.c *.h *.in $dir
cp -pf CMakeLists.txt LICENSE.txt Makefile $dir
cp -pf README.txt README-new.txt $dir
mkdir $dir/utils
cp -pf utils/chkcmake.sh $dir/utils
for d in mkconfig mkc_config man po; do
  cp -p -rf ${d} $dir
done

chmod -R a+r $dir
rm -f *.tar.gz
tar cf - $dir |
    gzip -9 > $dir.tar.gz

rm -rf $dir > /dev/null 2>&1

exit 0
