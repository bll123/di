#!/usr/bin/awk
#
#  Copyright 2025 Brad Lanam Pleasant Hill CA
#

BEGIN {
  sarr[0] = "";
  scount = 0;
  skip = 0;
}

function printarr() {
  if (scount == 2 && sarr[1] ~ /^< #define _proto_stdc/) {
    scount = 0;
  }
  if (scount == 2 && sarr[1] ~ "^---") {
    scount = 0;
  }
  if (scount == 1 && sarr[0] ~ "^[1-9]") {
    scount = 0;
  }
  for (i = 0; i < scount; ++i) {
    print sarr[i];
  }
  scount = 0;
  sarr[scount] = $0;
  ++scount;
}

{
#print "-- " $0;
  if ($0 ~ /^[01-9]/) {
#print "-- scount: " scount;
    printarr()
  } else {
    if ($0 ~ /^> \r?$/) {
      next;
    }
    if ($0 ~ /Copyright/) {
      scount = 0;
#print "-- reset-cr";
      next;
    }
    if ($0 ~ /DI_BUILD_SYS/) {
      scount = 0;
#print "-- reset-build-sys";
      next;
    }
    if ($0 ~ /^< #define _memberxdr_/) {
#print "-- skip-memberxdr";
      next;
    }
    if ($0 ~ /_args_getfsstat/) {
#print "-- next-getfsstat";
      next;
    }

    if ($0 ~ /^< #define _c_arg_[13-9]_quotactl/) {
      # mkconfig only
#print "-- next-c-arg-quotactl";
      next;
    } else if ($0 ~ /^> #define _c_arg_2_quotactl \r?$/) {
      # normal cmake output when no quotactl
      next;
    } else if ($0 ~ /^> #define _c_arg_2_getfsstat \r?$/) {
      # normal cmake output when no getfsstat
#print "-- skip-c-arg-2-getfstat-cmake";
      next;
    } else if ($0 ~ /^< #define _c_arg_[13-9]_getfsstat/) {
      # mkconfig only
#print "-- next-c-arg-getfsstat";
      next;
    } else if ($0 ~ /^. #define _c_arg_2_quotactl/) {
      ;
    } else if ($0 ~ /^< #define _c_arg_[1-9]_/) {
      # mkconfig only
#print "-- next-c-arg-";
      next;
    }

    if ($0 ~ /^> #define xdr_[a-z_]* \r?$/) {
      # normal cmake output when no xdr
#print "-- cmake: empty xdr";
      next;
    }

    if ($0 ~ /^< #define _c_type_/) {
      # mkconfig only
#print "-- next-c-type";
      next;
    }

    if ($0 ~ /^. #define DI_USE_MATH DI_/) {
      # remove this as they're in different places.
      next;
    }

    if ($0 ~ /^. #define DI_PREFIX /) {
      # remove this as they can be different places.
      next;
    }

    sarr[scount] = $0;
    ++scount;
  }
}

END {
  printarr()
}
