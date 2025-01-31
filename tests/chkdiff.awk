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
#print "-- " $0;
  if ($0 ~ /^[01-9]/) {
#print "-- scount: " scount;
    if (scount == 2 && sarr[1] ~ /^< #define _proto_stdc/) {
      scount = 0;
#print "-- reset-proto";
    }
    if (scount == 2 && sarr[1] ~ /^. #define DI_USE_MATH DI_/) {
      # remove this as they're in different places.
      scount = 0;
    }
    if (scount == 2 && sarr[1] ~ "^---") {
      scount = 0;
    }
    if (scount == 3 && sarr[2] ~ "^> #endif") {
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
    if ($0 ~ /_args_getfsstat/) {
#print "-- next-getfsstat";
      next;
    }
    if ($0 ~ /^> #if _command_msgfmt$/) {
      next;
    }
    if ($0 ~ /_cmd_loc_msgfmt/) {
      next;
    }

    if ($0 ~ /^< #define _c_arg_[13-9]_quotactl/) {
      # mkconfig only
#print "-- next-c-arg-quotactl";
      next;
    } else if ($0 ~ /^> #define _c_arg_2_quotactl $/) {
      # normal cmake output when no quotactl
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
  if (scount == 2 && sarr[1] ~ /^. #define DI_USE_MATH DI_/) {
    # remove this as they're in different places.
    scount = 0;
  }
  for (i = 0; i < scount; ++i) {
    print i ": " sarr[i];
  }
}
