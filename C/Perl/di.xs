#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include "config.h"
#include "dimain.h"
#include "di.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _sys_types \
    && ! defined (_DI_INC_SYS_TYPES_H) /* xenix */
# define _DI_INC_SYS_TYPES_H
# include <sys/types.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
#endif


#define DI_ARGV_SEP             " 	"  /* space, tab */
#define DI_MAX_ARGV             50

static SV *
diproc (pTHX_ char *args) {
  int               argscount;
  int               argc;
  const char        *argv [DI_MAX_ARGV];
  char              *tptr;
  int               i;
  diData_t          *diDataOut;
  char              *dispPtr;
  char              *rv;
  diDiskInfo_t      *diskInfo;
  HV                *rh;
  HV                *dh;
  SV                *RETVAL;

  tptr = strtok (args, DI_ARGV_SEP);
  argv[0] = "diskspace";
  argc = 1;
  while (tptr != (char *) NULL) {
    if (argc >= DI_MAX_ARGV) {
      break;
    }
    argv[argc++] = tptr;
    tptr = strtok ((char *) NULL, DI_ARGV_SEP);
  }
  if (tptr == (char *) NULL) {
    argc = 3;
    argv[1] = "-f";
    argv[2] = "";
    argv[3] = NULL;
  }

  rv = dimainproc (argc, argv, 1, &diDataOut);
  dispPtr = strtok (rv, "\n");

  rh = (HV *) sv_2mortal ((SV *) newHV());

  diskInfo = diDataOut->diskInfo;
  for (i = 0; i < diDataOut->count; ++i) {
    diDiskInfo_t    *dinfo;

    dinfo = &(diskInfo [diskInfo [i].sortIndex[DI_TOT_SORT_IDX]]);
    if (! dinfo->doPrint) {
      continue;
    }

    dh = newHV();
    /* dh = (HV *) sv_2mortal ((SV *) newHV()); */
    hv_store (dh, "device",           6, newSVpv (dinfo->special, 0), 0);
    hv_store (dh, "fstype",           6, newSVpv (dinfo->fsType, 0), 0);
    hv_store (dh, "total",            5, newSVnv (dinfo->totalSpace), 0);
    hv_store (dh, "free",             4, newSVnv (dinfo->freeSpace), 0);
    hv_store (dh, "available",        9, newSVnv (dinfo->availSpace), 0);
    hv_store (dh, "totalinodes",     11, newSVnv (dinfo->totalInodes), 0);
    hv_store (dh, "freeinodes",      10, newSVnv (dinfo->freeInodes), 0);
    hv_store (dh, "availableinodes", 15, newSVnv (dinfo->availInodes), 0);
    hv_store (dh, "mountoptions",    12, newSVpv (dinfo->options, 0), 0);
/* ### need to parse display and turn it into an array */
    hv_store (dh, "display",          7, newSVpv (dispPtr, 0), 0);
    hv_store (rh, dinfo->name, (I32) strlen (dinfo->name),
        newRV ((SV *) dh), 0);
    dispPtr = strtok ((char *) NULL, "\n");
  }

  RETVAL = newRV ((SV *) rh);
  cleanup (diDataOut);
  return RETVAL;
}


MODULE = Filesys::di		PACKAGE = Filesys::di

SV *
diskspace (char *args)
  PROTOTYPE: @
  CODE:
    RETVAL = diproc (aTHX_ args);
  OUTPUT:
    RETVAL

