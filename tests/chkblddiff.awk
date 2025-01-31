#!/usr/bin/awk
#
#  Copyright 2025 Brad Lanam Pleasant Hill CA
#

BEGIN {
  sarr[0] = "";
  scount = 0;
  skip = 0;
}

{
  if ($0 ~ /^[01-9]/) {
    if (scount == 1 && sarr[0] ~ "^[1-9]") {
      scount = 0;
    }
    for (i = 0; i < scount; ++i) {
      print sarr[i];
    }
    scount = 0;
    sarr[scount] = $0;
    ++scount;
  } else {
    if ($0 ~ /^> $/) {
      next;
    }
    if ($0 ~ /Created on/) {
      scount = 0;
      next;
    }
    if ($0 ~ /DI_PREFIX/) {
      scount = 0;
      next;
    }
    sarr[scount] = $0;
    ++scount;
  }
}

END {
  for (i = 0; i < scount; ++i) {
    print i ": " sarr[i];
  }
}
