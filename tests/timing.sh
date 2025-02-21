#!/bin/bash

function runtest {
  count=0
  while test $count -lt 1000; do
    ./x/bin/di -a > /dev/null
    count=$(($count+1))
  done
}

function dotest {
  typ=$1

  echo "== ${typ}"
  make distclean
  make PREFIX=$(pwd)/x DI_USE_MATH=${typ} > w 2>&1
  make PREFIX=$(pwd)/x DI_USE_MATH=${typ} install > ww 2>&1
  ./x/bin/di -a > /dev/null
  time runtest
}

systype=$(uname -s)

dotest DI_GMP
if [[ $systype == Linux ]]; then
  dotest DI_TOMMATH
fi
if [[ $systype == Darwin ]]; then
  dotest DI_MPDECIMAL
fi
dotest DI_INTERNAL
