#!/bin/sh

echo ${EN} "di compare mkconfig.sh mkconfig.pl${EC}" >&3

cd ..
make distclean
rm -f config.h.sh config.h.pl
./Build -mksh config.h
mv config.h config.h.sh
rm -f mkconfig.cache
./Build -mkpl config.h
mv config.h config.h.pl
diff -w config.h.sh config.h.pl
rc=$?
rm -f config.h.sh config.h.pl

exit $rc
