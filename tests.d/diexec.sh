#!/bin/sh

echo ${EN} "di executes${EC}" >&3

cd ..
./di 
rc=$?
exit $rc
