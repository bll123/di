#include "config.h"
#include "di.h"

#include <stdio.h>
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _sys_types
# include <sys/types.h>
#endif
#if _hdr_errno
# include <errno.h>
#endif
#if _sys_quota
# include <sys/quota.h>
#endif
#if _sys_fs_ufsquota
# include <sys/fs/ufs_quota.h>
#endif
#if _hdr_ufs_quota
# include <ufs/quota.h>
#endif
#if _hdr_ufs_ufs_quota
# include <ufs/ufs/quota.h>
#endif

#if defined(__cplusplus)
  extern "C" {
#endif

extern int debug;

#if defined(__cplusplus)
  }
#endif


void
#if _proto_stdc
diquotactl (diQuota_t *diqinfo)
#else
diquotactl (diqinfo)
  diQuota_t     *diqinfo;
#endif
{
  int           rc;
  struct dqblk  qinfo;
  _fs_size_t    tsize;

  diqinfo->hard = 0;
  diqinfo->used = 0;
  diqinfo->ihard = 0;
  diqinfo->iused = 0;

#if _lib_quotactl && _quotactl_special_pos == 1
  rc = quotactl (diqinfo->special, QCMD (Q_GETQUOTA, USRQUOTA),
        (int) diqinfo->uid, (caddr_t) &qinfo);
#endif
#if _lib_quotactl && _quotactl_special_pos == 2
  rc = quotactl (QCMD (Q_GETQUOTA, USRQUOTA), diqinfo->special,
        (int) diqinfo->uid, (caddr_t) &qinfo);
#endif
#if _hdr_fs_ufs_quota        /* Solaris */
  {
    int             fd;
    struct qoutctl  qop;

    qop.op = Q_GETQUOTA;
    qop.uid = diqinfo->uid;
    qop.addr = (caddr_t) &qinfo;
    fd = open (strcat (diqinfo->name, "/quotas"), O_RDONLY | O_NOCTTY);
    if (fd >= 0) {
      rc = ioctl (fd, Q_QUOTACTL, &qop);
      close (fd);
    } else {
      rc = fd;
    }
  }
#endif

  tsize = 0;
  if (rc == 0) {
    diqinfo->hard = qinfo.dqb_bhardlimit * BLOCK_SIZE / diqinfo->blockSize;
    diqinfo->ihard = qinfo.dqb_ihardlimit;
    diqinfo->iused = qinfo.dqb_curinodes;
#if _mem_dqb_curspace_dqblk
    diqinfo->used = qinfo.dqb_curspace / diqinfo->blockSize;
#endif
#if _mem_dqb_curblocks_dqblk
    diqinfo->used = qinfo.dqb_curblocks * BLOCK_SIZE / diqinfo->blockSize;
#endif
    if (debug > 2) {
      printf ("quota: %s used: %lld hard: %lld\n", diqinfo->name,
          diqinfo->used, diqinfo->hard);
      printf ("quota: orig hard: %lld\n", qinfo.dqb_bhardlimit);
    }
  } else {
    if (debug > 2) {
      printf ("quota: %s errno %d\n", diqinfo->name, errno);
    }
  }

#if _lib_quotactl
  if (rc == 0 || errno != ESRCH) {
# if _lib_quotactl && _quotactl_special_pos == 1
    rc = quotactl (diqinfo->special, QCMD (Q_GETQUOTA, GRPQUOTA),
          (int) diqinfo->uid, (caddr_t) &qinfo);
# endif
# if _lib_quotactl && _quotactl_special_pos == 2
    rc = quotactl (QCMD (Q_GETQUOTA, GRPQUOTA), diqinfo->special,
             (int) diqinfo->uid, (caddr_t) &qinfo);
# endif
    if (rc == 0) {
      tsize = qinfo.dqb_bhardlimit * BLOCK_SIZE / diqinfo->blockSize;
      if (debug > 2) {
        printf ("quota: group hard: %lld\n", tsize);
      }
      if (tsize != 0 && tsize < diqinfo->hard) {
        diqinfo->hard = tsize;
      }

#if _mem_dqb_curspace_dqblk
      tsize = qinfo.dqb_curspace / diqinfo->blockSize;
#endif
#if _mem_dqb_curblocks_dqblk
      tsize = qinfo.dqb_curblocks * BLOCK_SIZE / diqinfo->blockSize;
#endif
      if (debug > 2) {
        printf ("quota: group used: %lld\n", tsize);
      }
      if (tsize != 0 && tsize < diqinfo->used) {
        diqinfo->used = tsize;
      }

      tsize = qinfo.dqb_ihardlimit;
      if (tsize != 0 && tsize < diqinfo->ihard) {
        diqinfo->ihard = tsize;
      }

      tsize = qinfo.dqb_curinodes;
      if (tsize != 0 && tsize < diqinfo->iused) {
        diqinfo->iused = tsize;
      }
    }
  }
#endif
}
