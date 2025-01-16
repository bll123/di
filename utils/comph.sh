#!/bin/sh

for f in *.h; do
  cc -Ibuild $f
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== $f: fail"
  fi
  if [ -f $f.gch ]; then
    rm $f.gch
  fi
done
