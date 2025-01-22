/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#include "config.h"

#if _hdr_zone
# include <zone.h>
#endif

#include "di.h"
#include "dizone.h"
#include "distrutils.h"

di_zone_info_t *
di_initialize_zones (void)
{
  di_zone_info_t  *zinfo;

  zinfo = malloc (sizeof (di_zone_info_t));
  zinfo->zoneCount = 0;
  zinfo->zones = (di_zone_summ_t *) NULL;

#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
  zinfo->uid = geteuid ();

  {
    int             i;
    zoneid_t        *zids = (zoneid_t *) NULL;

    zinfo->myzoneid = getzoneid ();

    if (zone_list (zids, &zinfo->zoneCount) == 0) {
      if (zinfo->zoneCount > 0) {
        zids = malloc (sizeof (zoneid_t) * zinfo->zoneCount);
        if (zids == (zoneid_t *) NULL) {
          fprintf (stderr, "malloc failed in main () (1).  errno %d\n", errno);
          diopts->exitFlag = DI_EXIT_FAIL;
          return zinfo;
        }
        zone_list (zids, &zinfo->zoneCount);
        zinfo->zones = malloc (sizeof (di_zone_summ_t) *
                zinfo->zoneCount);
        if (zinfo->zones == (di_zone_summ_t *) NULL) {
          fprintf (stderr, "malloc failed in main () (2).  errno %d\n", errno);
          diopts->exitFlag = DI_EXIT_FAIL;
          return zinfo;
        }
      }
    }

    zinfo->globalIdx = 0;
    for (i = 0; i < (int) zinfo->zoneCount; ++i) {
      int     len;

      zinfo->zones [i].zoneid = zids [i];
      len = zone_getattr (zids [i], ZONE_ATTR_ROOT,
          zinfo->zones [i].rootpath, MAXPATHLEN);
      if (len >= 0) {
        zinfo->zones [i].rootpathlen = (Size_t) len;
        strncat (zinfo->zones [i].rootpath, "/", MAXPATHLEN);
        if (zinfo->zones [i].zoneid == 0) {
          zinfo->globalIdx = i;
        }

        len = zone_getattr (zids [i], ZONE_ATTR_NAME,
            zinfo->zones [i].name, ZONENAME_MAX);
        if (*diopts->zoneDisplay == '\0' &&
            zinfo->myzoneid == zinfo->zones [i].zoneid) {
          stpecpy (diopts->zoneDisplay,
              diopts->zoneDisplay + MAXPATHLEN, zinfo->zones [i].name);
        }
        if (debug > 4) {
          printf ("zone:%d:%s:%s:\n", (int) zinfo->zones [i].zoneid,
              zinfo->zones [i].name, zinfo->zones [i].rootpath);
        }
      }
    }

    free ( (void *) zids);
  }

  if (debug > 4) {
    printf ("zone:my:%d:%s:glob:%d:\n", (int) zinfo->myzoneid,
        zinfo->zoneDisplay, zinfo->globalIdx);
  }
#endif

  return zinfo;
}

void
di_free_zones (di_zone_info_t *zinfo)
{
  if (zinfo == NULL) {
    return;
  }

  if (zinfo->zones != (di_zone_summ_t *) NULL) {
    free (zinfo->zones);
  }
  free (zinfo);
}
