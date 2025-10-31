#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

LOG=comparemath.log
TMP=comparemath.tmp
grc=0

> $LOG

declare -A m
m=( DI_GMP g DI_MPDECIMAL m DI_TOMMATH t DI_INTERNAL i )

for mlib in ${!m[@]}; do
  tag=${m[$mlib]}

  test -d x.${tag} && rm -rf x.${tag}
  make -e DI_USE_MATH=${mlib} PREFIX=$(pwd)/x.${tag} >> $LOG 2>&1
  rc=$?
  if [[ $rc -ne 0 ]]; then
    echo "compile with $mlib failed"
    exit 1
  fi
  make -e DI_USE_MATH=${mlib} PREFIX=$(pwd)/x.${tag} install >> $LOG 2>&1
  rc=$?
  if [[ $rc -ne 0 ]]; then
    echo "install with $mlib failed"
    exit 1
  fi
  make -e DI_USE_MATH=${mlib} PREFIX=$(pwd)/x.${tag} test >> $LOG 2>&1
  rc=$?
  if [[ $rc -ne 0 ]]; then
    echo "test with $mlib failed"
    exit 1
  fi
done

for mlib in ${!m[@]}; do
  tag=${m[$mlib]}

  dir=./x.${tag}
  ${dir}/bin/di -a -d 1 > ${dir}/di.out.1
  ${dir}/bin/di -a -d m > ${dir}/di.out.m
  ${dir}/bin/di -a -d h > ${dir}/di.out.h
  ${dir}/bin/di -a -d H > ${dir}/di.out.H
done

# run again with all the di.out files in place so that the sizes
# will all be the same
for mlib in ${!m[@]}; do
  tag=${m[$mlib]}

  dir=./x.${tag}
  ${dir}/bin/di -a -d 1 > ${dir}/di.out.1
  ${dir}/bin/di -a -d m > ${dir}/di.out.m
  ${dir}/bin/di -a -d h > ${dir}/di.out.h
  ${dir}/bin/di -a -d H > ${dir}/di.out.H
done

count=0
ftag=""
for mlib in ${!m[@]}; do
  tag=${m[$mlib]}
  if [[ $count -eq 0 ]]; then
    fmlib=${mlib}
    ftag=${tag}
    count=$(($count+1))
    continue
  fi

  dir=./x.${tag}

  > ${dir}/diff.out
  for sfx in 1 m h H; do
    diff ./x.${ftag}/di.out.${sfx} ${dir}/di.out.${sfx} >> ${dir}/diff.out
  done

  dlc=$(cat ${dir}/diff.out | wc -l)
  rm -f $TMP
  if [[ $dlc -ne 0 ]]; then
    echo "${fmlib}, ${mlib} differ ${dlc}"
    grc=1
  fi

  count=$(($count+1))
done

if [[ $grc -eq 0 ]]; then
  for mlib in ${!m[@]}; do
    tag=${m[$mlib]}
    dir=./x.${tag}
    test -d ${dir} && rm -rf ${dir}
  done
fi

rm -f $LOG > /dev/null 2>&1
rm -f $TMP > /dev/null 2>&1

exit 0
