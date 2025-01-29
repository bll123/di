#!/usr/bin/awk
#
#  Copyright 2025 Brad Lanam Pleasant Hill CA
#

BEGIN {
  sarr[0] = "";
  scount = 0;
  start = 0;
  skip = 0;
}

{
#print "-- " $0;
  if ($0 ~ /^[01-9]/) {
#print "-- start nr:" NR " start: " start " scount: " scount;
    if (NR == start + 2 && sarr[1] ~ /^< #define _proto_stdc/) {
      scount = 0;
#print "-- reset-proto";
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
    start = NR;
#print "-- new start: " start;
  } else {
    if ($0 ~ /^> $/) {
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
    if ($0 ~ /^< #if . defined .DI_USE_MATH/) {
      skip = 1;
#print "-- beg-use-math";
      next;
    }
    if ($0 ~ /^< #endif....END DI_USE_MATH/) {
      scount = 0;
      skip = 0;
#print "-- reset-use-math";
      next;
    }
    if (skip == 1) {
#print "-- in-skip";
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
    } else if ($0 ~ /^> #define _c_arg_2_getfsstat $/) {
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

    if ($0 ~ /^> #define DI_USE_MATH DI_/) {
      # cmake output
      next;
    }
    if ($0 ~ /^< #define _c_type_/) {
      # mkconfig only
#print "-- next-c-type";
      next;
    }

    sarr[scount] = $0;
    ++scount;
  }
}

END {
  for (i = 0; i < scount; ++i) {
    print sarr[i];
  }
}
