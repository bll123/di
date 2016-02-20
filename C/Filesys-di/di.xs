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
  int               argc;
  const char        **argv;
  const char        *tptr;
  int               i;
  diData_t          *diDataOut;
  char              *display;
  diDiskInfo_t      *diskInfo;
  HV                *rh;
  HV                *dh;
  SV                *RETVAL;

  tptr = strtok (args, DI_ARGV_SEP);
  argc = 1;
  argv[0] = "";
  while (tptr != (char *) NULL)
  {
    if (argc >= DI_MAX_ARGV) {
      break;
    }
    argv[argc++] = tptr;
    tptr = strtok ((char *) NULL, DI_ARGV_SEP);
  }

  display = dimainproc (argc, argv, 1, &diDataOut);

  rh = (HV *) sv_2mortal ((SV *) newHV());

  diskInfo = diDataOut->diskInfo;
  for (i = 0; i < diDataOut->count; ++i) {
    diDiskInfo_t    *dinfo;

    dinfo = &(diskInfo [diskInfo [i].sortIndex[DI_TOT_SORT_IDX]]);
    if (! dinfo->doPrint) {
      continue;
    }

    dh = (HV *) sv_2mortal ((SV *) newHV());
    hv_store (dh, "device",           6, newSVpv (dinfo->special, 0), 0);
    hv_store (dh, "fstype",           6, newSVpv (dinfo->fsType, 0), 0);
    hv_store (dh, "total",            5, newSVuv (dinfo->totalSpace), 0);
    hv_store (dh, "free",             4, newSVuv (dinfo->freeSpace), 0);
    hv_store (dh, "available",        9, newSVuv (dinfo->availSpace), 0);
    hv_store (dh, "totalinodes",     11, newSVuv (dinfo->totalInodes), 0);
    hv_store (dh, "freeinodes",      10, newSVuv (dinfo->freeInodes), 0);
    hv_store (dh, "availableinodes", 15, newSVuv (dinfo->availInodes), 0);
    hv_store (dh, "mountoptions",    12, newSVpv (dinfo->options, 0), 0);
    hv_store (dh, "display",          7, newSVpv (display, 0), 0);
    hv_store (rh, dinfo->name, (I32) strlen (dinfo->name), (SV *) dh, 0);
  }

  RETVAL = newRV ((SV *) rh);
  cleanup (diDataOut);
  return RETVAL;
}


MODULE = Filesys::di		PACKAGE = Filesys::di

SV *
di (args)
    char *args
  CODE:
    RETVAL = diproc (aTHX_ args);
  OUTPUT:
    RETVAL

