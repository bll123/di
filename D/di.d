// written in the D programming language

import std.stdio;
import std.string;

import config;
import options;
import dispopts;
import diskpart;
import display;
import dilocale;
import diquota;

enum string FUSE_FS = "fuse";

void main (string[] args)
{
  Options       opts;
  DisplayOpts   dispOpts;

  initLocale ();
  getDIOptions (args, opts, dispOpts);

  auto dpList = new DiskPartitions (opts.debugLevel);
  dpList.getEntries ();
  auto hasPooled = preCheckDiskPartitions (dpList, opts);
  dpList.getPartitionInfo ();
  checkDiskQuotas (dpList, opts);
  checkDiskPartitions (dpList, opts);
  doDisplay (opts, dispOpts, dpList, hasPooled);
}

bool
preCheckDiskPartitions (ref DiskPartitions dpList, Options opts)
{
  bool      hasPooled;

  foreach (ref dp; dpList.diskPartitions)
  {
    if (dp.fsType == "zfs" || dp.fsType == "advfs")
    {
      dp.isPooledFS = true;
      hasPooled = true;
    }

    if (! dp.isRemote && dp.fsType[0..2] == "nfs") {
      dp.setRemote = true;
    }

    if (opts.localOnly && dp.isRemote)
    {
      dp.setPrintFlag = dp.DI_PRINT_IGNORE;
    }

//    static if (_lib_zone_list && _lib_getzoneid && _lib_zone_getattr) {
//      checkZone (dp, &diData->zoneInfo, opts.displayAll);
//    }

    checkIncludeList (dp, opts);

    if (opts.ignoreList.length > 0)
    {
      if (opts.ignoreList.get (dp.fsType, false) ||
          (dp.fsType[0..3] == FUSE_FS &&
          opts.ignoreList.get (FUSE_FS, false)))
      {
        dp.setPrintFlag = dp.DI_PRINT_EXCLUDE;
      }
    }
  }

  return hasPooled;
}

void
checkIncludeList (ref DiskPartition dp, Options opts)
{
  if (opts.includeList.length > 0)
  {
    if (dp.printFlag != dp.DI_PRINT_BAD &&
        dp.printFlag != dp.DI_PRINT_OUTOFZONE &&
        opts.includeList.get (dp.fsType, false) ||
        (dp.fsType[0..3] == FUSE_FS &&
         opts.includeList.get (FUSE_FS, false))) {
      dp.setDoPrint = true;
    }
    else {
      dp.setDoPrint = false;
    }
  }
}


void
checkDiskPartitions (ref DiskPartitions dpList, Options opts)
{
  foreach (ref dp; dpList.diskPartitions)
  {
    dp.setDoPrint = true;

    if (dp.printFlag == dp.DI_PRINT_EXCLUDE ||
        dp.printFlag == dp.DI_PRINT_BAD ||
        dp.printFlag == dp.DI_PRINT_OUTOFZONE) {
      dp.setDoPrint = false;
      /* -a flag does not affect these */
      continue;
    }

    if (dp.printFlag == dp.DI_PRINT_IGNORE ||
        dp.printFlag == dp.DI_PRINT_SKIP) {
      dp.setDoPrint = opts.displayAll;
      continue;
    }

    dp.checkPartSizes;

    if (dp.printFlag == dp.DI_PRINT_OK) {
      if (dp.totalBlocks <= 0.0) {
        dp.setPrintFlag = dp.DI_PRINT_IGNORE;
        dp.setDoPrint = opts.displayAll;
      }
    }

    checkIncludeList (dp, opts);
  }
}

/+

typedef struct {
    Uid_t           uid;
    zoneid_t        myzoneid;
    zoneSummary_t   *zones;
    Uint_t          zoneCount;
    char            zoneDisplay [MAXPATHLEN + 1];
    int             globalIdx;
} zoneInfo_t;

diData.zoneInfo.uid = geteuid ();
diData.zoneInfo.zoneDisplay [0] = '\0';
diData.zoneInfo.zoneCount = 0;
diData.zoneInfo.zones = (zoneSummary_t *) NULL;

// initialize zone info

#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
    {
        zoneid_t        *zids = (zoneid_t *) NULL;
        zoneInfo_t      *zi;

        zi = &diData.zoneInfo;
        zi->myzoneid = getzoneid ();

        if (zone_list (zids, &zi->zoneCount) == 0)
        {
            if (zi->zoneCount > 0)
            {
                zids = malloc (sizeof (zoneid_t) * zi->zoneCount);
                if (zids == (zoneid_t *) NULL)
                {
                    fprintf (stderr, "malloc failed in main() (1).  errno %d\n", errno);
                    exit (1);
                }
                zone_list (zids, &zi->zoneCount);
                zi->zones = malloc (sizeof (zoneSummary_t) *
                        zi->zoneCount);
                if (zi->zones == (zoneSummary_t *) NULL)
                {
                    fprintf (stderr, "malloc failed in main() (2).  errno %d\n", errno);
                    exit (1);
                }
            }
        }

        zi->globalIdx = 0;
        for (i = 0; i < (int) zi->zoneCount; ++i)
        {
            int     len;

            zi->zones[i].zoneid = zids[i];
            len = zone_getattr (zids[i], ZONE_ATTR_ROOT,
                    zi->zones[i].rootpath, MAXPATHLEN);
            if (len >= 0)
            {
                zi->zones[i].rootpathlen = (Size_t) len;
                strncat (zi->zones[i].rootpath, "/", MAXPATHLEN);
                if (zi->zones[i].zoneid == 0)
                {
                    zi->globalIdx = i;
                }

                len = zone_getattr (zids[i], ZONE_ATTR_NAME,
                        zi->zones[i].name, ZONENAME_MAX);
                if (*zi->zoneDisplay == '\0' &&
                    zi->myzoneid == zi->zones[i].zoneid)
                {
                    strncpy (zi->zoneDisplay, zi->zones[i].name, MAXPATHLEN);
                }
                if (debug > 4)
                {
                    printf ("zone:%d:%s:%s:\n", (int) zi->zones[i].zoneid,
                            zi->zones[i].name, zi->zones[i].rootpath);
                }
            }
        }

        free ((void *) zids);
    }

    if (debug > 4)
    {
        printf ("zone:my:%d:%s:glob:%d:\n", (int) diData.zoneInfo.myzoneid,
                diData.zoneInfo.zoneDisplay, diData.zoneInfo.globalIdx);
    }
#endif

// clean up zones
if (diData->zoneInfo.zones != (zoneSummary_t *) NULL)
{
  free ((void *) diData->zoneInfo.zones);
}


static if (_lib_zone_list && _lib_getzoneid && _lib_zone_getattr) {
checkZone (DiskPartition dp, zoneInfo_t *zoneInfo, bool displayAll)
{
    int         i;
    int         idx = { -1 };

    if (strcmp (zoneInfo->zoneDisplay, "all") == 0 &&
        zoneInfo->uid == 0)
    {
        return;
    }

    for (i = 0; i < (int) zoneInfo->zoneCount; ++i)
    {
        /* find the zone the filesystem is in, if non-global */
        if (debug > 5)
        {
            printf (" checkZone:%s:compare:%d:%s:\n",
                    diskInfo->name,
                    zoneInfo->zones[i].rootpathlen,
                    zoneInfo->zones[i].rootpath);
        }
        if (strncmp (zoneInfo->zones[i].rootpath,
             diskInfo->name, zoneInfo->zones[i].rootpathlen) == 0)
        {
            if (debug > 4)
            {
                printf (" checkZone:%s:found zone:%s:\n",
                        diskInfo->name, zoneInfo->zones[i].name);
            }
            idx = i;
            break;
        }
        if (idx == -1)
        {
            idx = zoneInfo->globalIdx;
        }
    }

        /* no root access, ignore any zones     */
        /* that don't match our zone id         */
        /* this will override any ignore flags  */
        /* already set                          */
    if (zoneInfo->uid != 0)
    {
        if (debug > 5)
        {
            printf (" checkZone:uid non-zero:chk zone:%d:%d:\n",
                    (int) zoneInfo->myzoneid,
                    (int) zoneInfo->zones[idx].zoneid);
        }
        if (zoneInfo->myzoneid != zoneInfo->zones[idx].zoneid)
        {
            if (debug > 4)
            {
                printf (" checkZone:not root, not zone:%d:%d:outofzone:\n",
                        (int) zoneInfo->myzoneid,
                        (int) zoneInfo->zones[idx].zoneid);
            }
            diskInfo->printFlag = DI_PRNT_OUTOFZONE;
        }
    }

    if (debug > 5)
    {
        printf (" checkZone:chk name:%s:%s:\n",
                zoneInfo->zoneDisplay, zoneInfo->zones[idx].name);
    }
        /* not the zone we want. ignore */
    if (! displayAll &&
        diskInfo->printFlag == DI_PRNT_OK &&
        strcmp (zoneInfo->zoneDisplay, "all") != 0 &&
        strcmp (zoneInfo->zoneDisplay,
                zoneInfo->zones[idx].name) != 0)
    {
        if (debug > 4)
        {
            printf (" checkZone:wrong zone:ignore:\n");
        }

        diskInfo->printFlag = DI_PRNT_IGNORE;
    }

        /* if displaying a non-global zone,   */
        /* don't display loopback filesystems */
    if (! displayAll &&
        diskInfo->printFlag == DI_PRNT_OK &&
        strcmp (zoneInfo->zoneDisplay, "global") != 0 &&
        diskInfo->isLoopback)
    {
        if (debug > 4)
        {
            printf (" checkZone:non-global/lofs:ignore:\n");
        }

        diskInfo->printFlag = DI_PRNT_IGNORE;
    }

    return;
}
} /* static if */
+/
