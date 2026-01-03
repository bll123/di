/* Copyright 2025-2026 Brad Lanam Pleasant Hill CA */

#include "config.h"

#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
#endif
#if _hdr_memory
# include <memory.h>
#endif
#if _hdr_errno
# include <errno.h>
#endif
#if _hdr_unistd
# include <unistd.h>
#endif
#if _hdr_zone
# include <zone.h>
#endif

#include "di.h"
#include "dizone.h"
#include "distrutils.h"
#include "dioptions.h"

di_zone_info_t *
di_initialize_zones (di_opt_t *diopts)
{
  di_zone_info_t  *zinfo;

  zinfo = (di_zone_info_t *) malloc (sizeof (di_zone_info_t));
  zinfo->zoneCount = 0;
  zinfo->zones = (di_zone_summ_t *) NULL;

#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
  zinfo->uid = geteuid ();

  {
    int             i;
    zoneid_t        *zids = (zoneid_t *) NULL;
    char            *rpp;
    char            *rpend;

    zinfo->myzoneid = getzoneid ();

    if (zone_list (zids, &zinfo->zoneCount) == 0) {
      if (zinfo->zoneCount > 0) {
        zids = (zoneid_t *) malloc (sizeof (zoneid_t) * zinfo->zoneCount);
        if (zids == (zoneid_t *) NULL) {
          fprintf (stderr, "malloc failed in main () (1).  errno %d\n", errno);
          return zinfo;
        }
        zone_list (zids, &zinfo->zoneCount);
        zinfo->zones = (di_zone_summ_t *) malloc (sizeof (di_zone_summ_t) *
                zinfo->zoneCount);
        if (zinfo->zones == (di_zone_summ_t *) NULL) {
          fprintf (stderr, "malloc failed in main () (2).  errno %d\n", errno);
          return zinfo;
        }
      }
    }

    zinfo->globalIdx = 0;
    for (i = 0; i < (int) zinfo->zoneCount; ++i) {
      int     len;

      zinfo->zones [i].zoneid = zids [i];
      len = (int) zone_getattr (zids [i], ZONE_ATTR_ROOT,
          zinfo->zones [i].rootpath, MAXPATHLEN);
      /* solaris: the length returned includes the null byte */
      if (len >= 0) {
        len -= 1;
        zinfo->zones [i].rootpathlen = (Size_t) len;
        if (zinfo->zones [i].zoneid != 0) {
          rpp = zinfo->zones [i].rootpath + len;
          rpend = zinfo->zones [i].rootpath + MAXPATHLEN;
          rpp = stpecpy (rpp, rpend, "/");
        }
        if (zinfo->zones [i].zoneid == 0) {
          zinfo->globalIdx = i;
        }

        len = (int) zone_getattr (zids [i], ZONE_ATTR_NAME,
            zinfo->zones [i].name, ZONENAME_MAX);
        if (*diopts->zoneDisplay == '\0' &&
            zinfo->myzoneid == zinfo->zones [i].zoneid) {
          stpecpy (diopts->zoneDisplay,
              diopts->zoneDisplay + MAXPATHLEN, zinfo->zones [i].name);
        }
        if (diopts->optval [DI_OPT_DEBUG] > 4) {
          printf ("zone:%d:%s:%s:%d\n", (int) zinfo->zones [i].zoneid,
              zinfo->zones [i].name, zinfo->zones [i].rootpath,
              (int) zinfo->zones [i].rootpathlen);
        }
      }
    }

    free ((void *) zids);
  }

  if (diopts->optval [DI_OPT_DEBUG] > 4) {
    printf ("zone:my:%d:%s:glob:%d:\n", (int) zinfo->myzoneid,
        diopts->zoneDisplay, zinfo->globalIdx);
  }
#endif

  return zinfo;
}

void
di_free_zones (di_zone_info_t *zinfo)
{
  if (zinfo == (di_zone_info_t *) NULL) {
    return;
  }

  if (zinfo->zones != (di_zone_summ_t *) NULL) {
    free (zinfo->zones);
  }
  free (zinfo);
}
