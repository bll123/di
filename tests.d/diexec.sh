#!/bin/sh

echo ${EN} "di executes${EC}" >&3

../di 
rc=$?
exit $rc
