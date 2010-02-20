#!/bin/sh

echo ${EN} "di nls${EC}" >&3

cd ..

./Build hasnls
./hasnls
hasnls=$?

if [ ${hasnls} -ne 0 ];then
  echo "no nls support; skipped"
  exit 0
fi

grc=1
for l in "de_DE" "de_DE.utf-8" "de_DE.UTF-8"; do
  LC_ALL="${l}" ./di -A | egrep Benutzt >/dev/null 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then
    grc=0
  fi
done

#exit $grc
# cannot depend on german being installed...
exit 0
