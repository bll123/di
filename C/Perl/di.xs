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

static AV *
parsedisp (pTHX_ char *val) {
  AV    *a;
  char  *dptr;

  a = newAV();
  dptr = strtok (val, "	"); /* tab character */
  while (dptr != (char *) NULL) {
    av_push (a, newSVpv (dptr, 0));
    dptr = strtok (NULL, "	"); /* tab character */
  }
  return a;
}

static SV *
diproc (pTHX_ char *args) {
  int               argscount;
  int               argc;
  const char        *argv [DI_MAX_ARGV];
  char              *tptr;
  char              **dispargs;
  int               i;
  diData_t          *diDataOut;
  char              *rv;
  diDiskInfo_t      *diskInfo;
  diDiskInfo_t      *dinfo;
  HV                *rh;
  HV                *dh;
  SV                *RETVAL;

  argv[0] = "diskspace";
  if (args == (char *) NULL || strcmp (args, "") == 0) {
    argc = 3;
    argv[1] = "-f";
    argv[2] = "";
    argv[3] = NULL;
  } else {
    tptr = strtok (args, DI_ARGV_SEP);
    argc = 1;
    while (tptr != (char *) NULL) {
      if (argc >= DI_MAX_ARGV) {
        break;
      }
      argv[argc++] = tptr;
      tptr = strtok ((char *) NULL, DI_ARGV_SEP);
    }
  }

  rv = dimainproc (argc, argv, 1, &diDataOut);
  diskInfo = diDataOut->diskInfo;

  dispargs = (char **) malloc (sizeof(char *) *
      (Size_t) diDataOut->count);
  if (rv != (char *) NULL) {
    tptr = strtok (rv, "\n");
  } else {
    tptr = (char *) NULL;
  }
  for (i = 0; i < diDataOut->count; ++i) {
    dinfo = &(diskInfo [diskInfo [i].sortIndex[DI_TOT_SORT_IDX]]);
    if (! dinfo->doPrint) {
      continue;
    }
    dispargs[i] = tptr;
    if (rv != (char *) NULL) {
      tptr = strtok (NULL, "\n");
    }
  }

  rh = (HV *) sv_2mortal ((SV *) newHV());
  for (i = 0; i < diDataOut->count; ++i) {
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
    if (dispargs[i] != (char *) NULL) {
      hv_store (dh, "display",          7,
          newRV ((SV *) parsedisp (aTHX_ dispargs[i])), 0);
    }
    hv_store (rh, dinfo->name, (I32) strlen (dinfo->name),
        newRV ((SV *) dh), 0);
  }

  RETVAL = newRV ((SV *) rh);
  free (rv);
  free (dispargs);
  cleanup (diDataOut);
  return RETVAL;
}


MODULE = Filesys::di		PACKAGE = Filesys::di

SV *
diskspace (char *args)
  PROTOTYPE: $
  CODE:
    RETVAL = diproc (aTHX_ args);
  OUTPUT:
    RETVAL

