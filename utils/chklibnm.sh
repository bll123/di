#!/bin/sh
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

rc=1
if [ -f /etc/os-release ]; then
  grep -l openSUSE /etc/os-release > /dev/null 2>&1
  rc=$?
  # there is another os that uses lib64, but I don't recall which one
  # at the moment...
  if [ $rc -ne 0 ]; then
    grep -l 'Calculate Linux' /etc/os-release > /dev/null 2>&1
    rc=$?
  fi
  if [ $rc -ne 0 ]; then
    grep -l 'Arch Linux' /etc/os-release > /dev/null 2>&1
    rc=$?
  fi
fi

if [ $rc -eq 0 ]; then
  echo lib64
else
  echo lib
fi
exit 0
