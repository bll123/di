#include "config.h"
#include "di.h"

#include <stdio.h>
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_unistd
# include <unistd.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
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
#if _sys_fs_ufs_quota
# include <sys/fs/ufs_quota.h>
#endif
#if _hdr_ufs_quota
# include <ufs/quota.h>
#endif
#if _hdr_ufs_ufs_quota
# include <ufs/ufs/quota.h>
#endif
  /* AIX doesn't seem to have quotactl declared.... */
  /* use their compatibility routine.               */
#if _hdr_linux_quota && ((! _sys_quota) || (_include_quota))
# include <linux/quota.h>
#endif

#if defined(__cplusplus)
  extern "C" {
#endif

extern int debug;

#if defined(__cplusplus)
  }
#endif

#ifdef BLOCK_SIZE           /* linux */
# define DI_QUOT_BLOCK_SIZE BLOCK_SIZE
#else
# ifdef DEV_BSIZE           /* tru64, et. al. */
#  define DI_QUOT_BLOCK_SIZE DEV_BSIZE
# else
#  define DI_QUOT_BLOCK_SIZE 512
# endif
#endif

  /* rename certain structure members for portability */
#ifdef _mem_dqb_fsoftlimit_dqblk
# define dqb_fsoftlimit dqb_isoftlimit
#endif
#ifdef _mem_dqb_fhardlimit_dqblk
# define dqb_fhardlimit dqb_ihardlimit
#endif
#ifdef _mem_dqb_curfiles_dqblk
# define dqb_curfiles dqb_curinodes
#endif

 /* ugh...the interface is not the same...use mount name instead of special */
#if _AIX || OSF1
# define special name
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
#if 0
  struct dQBlk64 qinfo64;
#endif
  char          *qiptr;
  _fs_size_t    tsize;
  _fs_size_t    qsize;

  qiptr = (char *) &qinfo;
  diqinfo->limit = 0;
  diqinfo->used = 0;
  diqinfo->ilimit = 0;
  diqinfo->iused = 0;

  rc = -1;

  qsize = 0;
#if _lib_quotactl && defined (Q_QUOTAINFO)
# if _lib_quotactl && _quotactl_special_pos == 1
  rc = quotactl (diqinfo->special, QCMD(Q_QUOTAINFO, 0),
        (int) diqinfo->uid, (caddr_t) &qsize);
# endif
# if _lib_quotactl && _quotactl_special_pos == 2
  rc = quotactl (QCMD(Q_QUOTAINFO, 0), diqinfo->special,
        (int) diqinfo->uid, (caddr_t) &qsize);
# endif
#endif
  if (debug > 2) {
    printf ("quota: size: %lld\n", qsize);
  }

#if _lib_quotactl && _quotactl_special_pos == 1
  rc = quotactl (diqinfo->special, QCMD (Q_GETQUOTA, USRQUOTA),
        (int) diqinfo->uid, (caddr_t) qiptr);
#endif
#if _lib_quotactl && _quotactl_special_pos == 2
  rc = quotactl (QCMD (Q_GETQUOTA, USRQUOTA), diqinfo->special,
        (int) diqinfo->uid, (caddr_t) qiptr);
#endif
#if _sys_fs_ufs_quota        /* Solaris */
  {
    int             fd;
    struct quotctl  qop;

    qop.op = Q_GETQUOTA;
    qop.uid = diqinfo->uid;
    qop.addr = (caddr_t) qiptr;
    fd = open (strcat (diqinfo->name, "/quotas"), O_RDONLY | O_NOCTTY);
    if (fd >= 0) {
      rc = ioctl (fd, Q_QUOTACTL, &qop);
      close (fd);
    } else {
      rc = fd;
    }
  }
#endif

  if (rc == 0) {
#if defined (QIF_BLIMITS)
    if ((qinfo.dqb_valid & QIF_BLIMITS) == QIF_BLIMITS) {
#endif
      diqinfo->limit = qinfo.dqb_bhardlimit * DI_QUOT_BLOCK_SIZE / diqinfo->blockSize;
      tsize = qinfo.dqb_bsoftlimit * DI_QUOT_BLOCK_SIZE / diqinfo->blockSize;
      if (tsize != 0 && tsize < diqinfo->limit) {
        if (debug > 2) {
          printf ("quota: using user soft: %lld\n", tsize);
        }
        diqinfo->limit = tsize;
      }
#if defined (QIF_BLIMITS)
    }
#endif
#if defined (QIF_ILIMITS)
    if ((qinfo.dqb_valid & QIF_ILIMITS) == QIF_ILIMITS) {
#endif
      diqinfo->ilimit = qinfo.dqb_ihardlimit;
#if defined (QIF_ILIMITS)
    }
#endif
#if defined (QIF_INODES)
    if ((qinfo.dqb_valid & QIF_INODES) == QIF_INODES) {
#endif
    diqinfo->iused = qinfo.dqb_curinodes;
#if defined (QIF_INODES)
    }
#endif

#if _mem_dqb_curspace_dqblk
# if defined (QIF_SPACE)
    if ((qinfo.dqb_valid & QIF_SPACE) == QIF_SPACE) {
# endif
      diqinfo->used = qinfo.dqb_curspace / diqinfo->blockSize;
# if defined (QIF_SPACE)
    }
# endif
#endif
#if _mem_dqb_curblocks_dqblk
    diqinfo->used = qinfo.dqb_curblocks * DI_QUOT_BLOCK_SIZE / diqinfo->blockSize;
#endif
    if (debug > 2) {
      printf ("quota: %s used: %lld limit: %lld\n", diqinfo->name,
          diqinfo->used, diqinfo->limit);
      printf ("quota: orig hard: %lld\n", qinfo.dqb_bhardlimit);
    }
  } else {
    if (debug > 2) {
      printf ("quota: %s errno %d\n", diqinfo->name, errno);
    }
  }

#if _lib_quotactl
  if (rc == 0 || errno != ESRCH) {
    tsize = 0;
# if _lib_quotactl && _quotactl_special_pos == 1
    rc = quotactl (diqinfo->special, QCMD (Q_GETQUOTA, GRPQUOTA),
          (int) diqinfo->uid, (caddr_t) qiptr);
# endif
# if _lib_quotactl && _quotactl_special_pos == 2
    rc = quotactl (QCMD (Q_GETQUOTA, GRPQUOTA), diqinfo->special,
             (int) diqinfo->uid, (caddr_t) qiptr);
# endif
    if (rc == 0) {
      tsize = qinfo.dqb_bhardlimit * DI_QUOT_BLOCK_SIZE / diqinfo->blockSize;
      if (debug > 2) {
        printf ("quota: group hard: %lld\n", tsize);
      }
      if (tsize != 0 && tsize < diqinfo->limit) {
        diqinfo->limit = tsize;
      }

      tsize = qinfo.dqb_bsoftlimit * DI_QUOT_BLOCK_SIZE / diqinfo->blockSize;
      if (debug > 2) {
        printf ("quota: group soft: %lld\n", tsize);
      }
      if (tsize != 0 && tsize < diqinfo->limit) {
        diqinfo->limit = tsize;
      }

#if _mem_dqb_curspace_dqblk
      tsize = qinfo.dqb_curspace / diqinfo->blockSize;
#endif
#if _mem_dqb_curblocks_dqblk
      tsize = qinfo.dqb_curblocks * DI_QUOT_BLOCK_SIZE / diqinfo->blockSize;
#endif
      if (debug > 2) {
        printf ("quota: group used: %lld\n", tsize);
      }
      if (tsize != 0 && tsize < diqinfo->used) {
        diqinfo->used = tsize;
      }

      tsize = qinfo.dqb_ihardlimit;
      if (tsize != 0 && tsize < diqinfo->ilimit) {
        diqinfo->ilimit = tsize;
      }

      tsize = qinfo.dqb_curinodes;
      if (tsize != 0 && tsize < diqinfo->iused) {
        diqinfo->iused = tsize;
      }
    }
  }
#endif
}
