#!/bin/sh
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

libnm=lib

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
  if [ $rc -ne 0 ]; then
    grep -l '^ID_LIKE=arch' /etc/os-release > /dev/null 2>&1
    rc=$?
  fi
fi

if [ $rc -eq 0 ]; then
  libnm=lib64
  if [ -h /usr/lib64 -a `readlink /usr/lib64` = lib ]; then
    # possibly an arch system, and /usr/lib64 points to /usr/lib
    libnm=lib
  fi
fi

# HP-UX ia64 uses lib/hpux64
case `uname -s` in
  HP-UX)
    case `uname -m` in
      ia64)
        libnm=lib/hpux64
        ;;
    esac
    ;;
esac

echo $libnm

exit 0
