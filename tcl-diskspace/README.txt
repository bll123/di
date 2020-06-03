di:
  https://sourceforge.net/projects/diskinfo-di/

Tcl Wiki Page:
  https://wiki.tcl-lang.org/page/Filesystem+Usage+%2F+Information%3A+di+Tcl+extension

Notes:
  Binaries for linux/32, linux/64, MacOS, windows/32 and windows/64 are
  included.

  The windows binaries were built using msys2, requiring that the
  windows API be used.   If you want a more unix style display,
  grab the 'di' program and build using cygwin.

Examples:

  package require diskspace

  % set di [diskspace -f {} /] ; # get only the basic information for root
  filesystem / {device /dev/sda1 fstype ext4 total 485893496832 free 269818650624 available 245113102336 totalinodes 30138368 freeinodes 29762805 availableinodes 29762805 mountoptions rw,errors=remount-ro}

  % set di [diskspace -f {up} /] ; # also get the used space and percentage used.
  / {display {201.2G      50%} device /dev/sda1 fstype ext4 total 485893496832 free 269818736640 available 245113188352 totalinodes 30138368 freeinodes 29762805 availableinodes 29762805 mountoptions rw,errors=remount-ro}
  % lassign [dict get $di / display] useddisplay percused

  % set di [diskspace -d 1 -f up /] ; # instead, get the used value in bytes.
  / {display {216074854400        50%} device /dev/sda1 fstype ext4 total 485893496832 free 269818642432 available 245113094144 totalinodes 30138368 freeinodes 29762805 availableinodes 29762805 mountoptions rw,errors=remount-ro}
  % lassign [dict get $di / display] usedbytes percused

  % set di [diskspace -f {}] ; # get information on the usual list of filesystems.
