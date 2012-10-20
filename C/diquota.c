/*
 * $Id$
 * $Source$
 *
 * Copyright 2011 Brad Lanam, Walnut Creek, CA
 */

#include "config.h"
#include "di.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_unistd
# include <unistd.h>
#endif
#if _sys_param
# include <sys/param.h>
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
#if _hdr_time
# include <time.h>
#endif
#if _sys_time && _inc_conflict__hdr_time__sys_time
# include <sys/time.h>
#endif
#if _hdr_quota
# include <quota.h>
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
#if _hdr_linux_dqblk_xfs
# include <linux/dqblk_xfs.h>
#endif
#if _hdr_jfs_quota
# include <jfs/quota.h>
#endif
  /* AIX 5.1 doesn't seem to have quotactl declared.... */
  /* use their compatibility routine.                   */
#if ! _args_quotactl && _hdr_linux_quota && \
      _inc_conflict__sys_quota__hdr_linux_quota
# include <linux/quota.h>
#endif
#if _hdr_rpc_rpc
# include <rpc/rpc.h>
#endif
#if _hdr_rpcsvc_rquota
# include <rpcsvc/rquota.h>
#endif

#if defined (__cplusplus) || defined (c_plusplus)
  extern "C" {
#endif

#if _has_std_nfs_quotas && ! _lib_quota_open
static bool_t xdr_quota_get _((XDR *, struct getquota_args *));
static bool_t xdr_quota_rslt _((XDR *, struct getquota_rslt *));
static void diquota_nfs _((diQuota_t *));
#endif

#if _has_std_quotas

typedef union {
  int               val;
# if _typ_struct_dqblk
  struct dqblk      qinfo;
# endif
# if _typ_struct_ufs_dqblk
  struct ufs_dqblk  ufsqinfo;
# endif
# if _typ_fs_disk_quota_t
  fs_disk_quota_t   xfsqinfo;
# endif
# if _typ_struct_quotaval
  struct {
    struct quotaval   qbval;
    struct quotaval   qival;
  } qval;
# endif
} qdata_t;

static void di_process_quotas _((char *, diQuota_t *, int, int, qdata_t *));
#endif

extern int debug;

#if defined (__cplusplus) || defined (c_plusplus)
  }
#endif

#ifdef BLOCK_SIZE           /* linux */
# define DI_QUOT_BLOCK_SIZE BLOCK_SIZE
#else
# ifdef DQBSIZE             /* AIX */
#  define DI_QUOT_BLOCK_SIZE DQBSIZE
# else
#  ifdef DEV_BSIZE           /* tru64, et. al. */
#   define DI_QUOT_BLOCK_SIZE DEV_BSIZE
#  else
#   define DI_QUOT_BLOCK_SIZE 512
#  endif
# endif
#endif

  /* rename certain structure members for portability       */
  /* it make the code below cleaner, but it's a bit more    */
  /* difficult to read it                                   */
#if _mem_struct_dqblk_dqb_fsoftlimit
# define dqb_isoftlimit dqb_fsoftlimit
#endif
#if _mem_struct_dqblk_dqb_fhardlimit
# define dqb_ihardlimit dqb_fhardlimit
#endif
#if _mem_struct_dqblk_dqb_curfiles
# define dqb_curinodes dqb_curfiles
#endif

void
#if _proto_stdc
diquota (diQuota_t *diqinfo)
#else
diquota (diqinfo)
  diQuota_t     *diqinfo;
#endif
{
  int               rc;
  int               xfsflag;
#if _has_std_quotas
  qdata_t           qdata;
#endif
#if _lib_quota_open
  struct quotahandle    *qh;
  struct quotakey       qkey;
#endif
#if _has_std_quotas && ! _lib_quota_open
  int               ucmd;
  int               gcmd;
#endif

  if (debug > 5) {
    printf ("quota: diquota\n");
  }
  rc = -1;
  xfsflag = FALSE;

  diqinfo->limit = 0;
  diqinfo->used = 0;
  diqinfo->ilimit = 0;
  diqinfo->iused = 0;

#if _lib_quota_open
  qh = quota_open (diqinfo->name);
#endif
#if _lib_quota_open
  rc = -3;
  if (qh != (struct quotahandle *) NULL) {
    memset (&qkey, 0, sizeof (struct quotakey));
    qkey.qk_idtype = QUOTA_IDTYPE_USER;
    qkey.qk_id = diqinfo->uid;
    qkey.qk_objtype = QUOTA_OBJTYPE_BLOCKS;
    rc = quota_get (qh, &qkey, &qdata.qval.qbval);
    if (rc == 0) {
      qkey.qk_objtype = QUOTA_OBJTYPE_FILES;
      rc = quota_get (qh, &qkey, &qdata.qval.qival);
    }
  }
#else
  if (strncmp (diqinfo->type, "nfs", (Size_t) 3) == 0 &&
      strcmp (diqinfo->type, "nfsd") != 0) {
# if _has_std_nfs_quotas
    diquota_nfs (diqinfo);
# endif
    return;
  }
  if (strcmp (diqinfo->type, "xfs") == 0) {
# if _hdr_linux_dqblk_xfs
    ucmd = QCMD (Q_XGETQUOTA, USRQUOTA);
    gcmd = QCMD (Q_XGETQUOTA, GRPQUOTA);
    xfsflag = TRUE;
# endif
    ;
  } else {
# if _define_QCMD
    ucmd = QCMD (Q_GETQUOTA, USRQUOTA);
    gcmd = QCMD (Q_GETQUOTA, GRPQUOTA);
# else
    /* hp-ux doesn't have QCMD */
    ucmd = Q_GETQUOTA;
    gcmd = Q_GETQUOTA;
# endif
  }

# if defined (__FreeBSD__) && __FreeBSD__ == 5
    /* quotactl on devfs fs panics the system (FreeBSD 5.1) */
  if (strcmp (diqinfo->type, "ufs") != 0) {
    return;
  }
# endif

  if (debug > 5) {
    printf ("quota: quotactl on %s (%d %d)\n", diqinfo->name, _quotactl_pos_1, _quotactl_pos_2);
  }
# if _lib_quotactl && _quotactl_pos_1
  rc = quotactl (diqinfo->name, ucmd,
        (int) diqinfo->uid, (caddr_t) &qdata.val);
# endif
# if _lib_quotactl && (_quotactl_pos_2 || _AIX)
#  if _AIX
  /* AIX has linux compatibility routine, but need name rather than special */
  rc = quotactl (ucmd, diqinfo->name,
        (int) diqinfo->uid, (caddr_t) &qdata.val);
#  else
  rc = quotactl (ucmd, diqinfo->special,
        (int) diqinfo->uid, (caddr_t) &qdata.val);
#  endif
# endif
# if _has_std_quotas && _sys_fs_ufs_quota        /* Solaris */
  {
    int             fd;
    struct quotctl  qop;
    char            tname [DI_NAME_LEN + 1];

    qop.op = Q_GETQUOTA;
    qop.uid = diqinfo->uid;
    qop.addr = (caddr_t) &qdata.qinfo;
    strncpy (tname, diqinfo->name, DI_NAME_LEN);
    strncat (tname, "/quotas", DI_NAME_LEN);
    fd = open (tname, O_RDONLY | O_NOCTTY);
    if (fd >= 0) {
      rc = ioctl (fd, Q_QUOTACTL, &qop);
      close (fd);
    } else {
      rc = fd;
    }
  }
# endif  /* _sys_fs_ufs_quota */
#endif /* else _lib_quota_open */

#if _has_std_quotas
  di_process_quotas ("usr", diqinfo, rc, xfsflag, &qdata);

# if _lib_quota_open
  rc = -3;
  if (qh != (struct quotahandle *) NULL) {
    memset (&qkey, 0, sizeof (struct quotakey));
    qkey.qk_idtype = QUOTA_IDTYPE_GROUP;
    qkey.qk_id = diqinfo->gid;
    qkey.qk_objtype = QUOTA_OBJTYPE_BLOCKS;
    rc = quota_get (qh, &qkey, &qdata.qval.qbval);
    if (rc == 0) {
      qkey.qk_objtype = QUOTA_OBJTYPE_FILES;
      rc = quota_get (qh, &qkey, &qdata.qval.qival);
    }
  }
# else
#  ifdef GRPQUOTA
  if (rc == 0 || errno != ESRCH) {
#   if _lib_quotactl && _quotactl_pos_1
    rc = quotactl (diqinfo->name, gcmd,
          (int) diqinfo->gid, (caddr_t) &qdata.val);
#   endif
#   if _lib_quotactl && _quotactl_pos_2
#    if _AIX  /* AIX has linux compatibility routine, but still need name */
    rc = quotactl (gcmd, diqinfo->name,
            (int) diqinfo->gid, (caddr_t) &qdata.val);
#    else
    rc = quotactl (gcmd, diqinfo->special,
            (int) diqinfo->gid, (caddr_t) &qdata.val);
#    endif /* ! aix */
#   endif /* quotactl_pos_2 */
  }
#  endif /* ifdef GRPQUOTA */
# endif /* else _lib_quota_open */
# if _lib_quota_open
  if (qh != (struct quotahandle *) NULL) {
    quota_close (qh);
  }
# endif

# if defined(GRPQUOTA) || _lib_quota_open
  di_process_quotas ("grp", diqinfo, rc, xfsflag, &qdata);
# endif
#endif /* _has_std_quotas */
}

#if _has_std_nfs_quotas && ! _lib_quota_open

#ifdef RQ_PATHLEN
# define DI_RQ_PATHLEN  RQ_PATHLEN
#else
# define DI_RQ_PATHLEN  1024
#endif

static bool_t
# if _proto_stdc
xdr_quota_get (XDR *xp, struct getquota_args *args)
# else
xdr_quota_get (xp, args)
  XDR                   *xp;
  struct getquota_args  *args;
# endif
{
  if (debug > 5) {
    printf ("quota: xdr_quota_get\n");
  }

  if (! xdr_string (xp, &args->gqa_pathp, DI_RQ_PATHLEN)) {
    return 0;
  }
  if (! xdr_gqa_uid (xp, &args->gqa_uid)) {
    return 0;
  }
  return 1;
}

static bool_t
# if _proto_stdc
xdr_quota_rslt (XDR *xp, struct getquota_rslt *rslt)
# else
xdr_quota_rslt (xp, rslt)
  XDR                   *xp;
  struct getquota_rslt  *rslt;
# endif
{
  int           quotastat;
  struct rquota *rptr;

  if (debug > 5) {
    printf ("quota: xdr_quota_rslt\n");
  }

  if (! xdr_int (xp, &quotastat)) {
    return 0;
  }
# if _mem_struct_getquota_rslt_gqr_status
  rslt->gqr_status = quotastat;
# else
  rslt->status = quotastat;
# endif
# if _mem_struct_getquota_rslt_gqr_rquota
  rptr = &rslt->gqr_rquota;
# else
  rptr = &rslt->getquota_rslt_u.gqr_rquota;
# endif

  if (! xdr_int (xp, &rptr->rq_bsize)) {
    return 0;
  }
  if (! xdr_bool (xp, &rptr->rq_active)) {
    return 0;
  }
  if (! xdr_rq_bhardlimit (xp, &rptr->rq_bhardlimit)) {
    return 0;
  }
  if (! xdr_rq_bsoftlimit (xp, &rptr->rq_bsoftlimit)) {
    return 0;
  }
  if (! xdr_rq_curblocks (xp, &rptr->rq_curblocks)) {
    return 0;
  }
  if (! xdr_rq_fhardlimit (xp, &rptr->rq_fhardlimit)) {
    return 0;
  }
  if (! xdr_rq_fsoftlimit (xp, &rptr->rq_fsoftlimit)) {
    return 0;
  }
  if (! xdr_rq_curfiles (xp, &rptr->rq_curfiles)) {
    return 0;
  }
  return (1);
}

static void
# if _proto_stdc
diquota_nfs (diQuota_t *diqinfo)
# else
diquota_nfs (diqinfo)
  diQuota_t     *diqinfo;
# endif
{
    CLIENT                  *rqclnt;
    enum clnt_stat          clnt_stat;
    struct timeval          timeout;
    char                    host [DI_SPEC_NAME_LEN + 1];
    char                    *ptr;
    char                    *path;
    struct getquota_args    args;
    struct getquota_rslt    result;
    struct rquota           *rptr;
    int                     quotastat;
    _fs_size_t              tsize;
    _fs_size_t              tblksize;

    if (debug > 5) {
      printf ("quota: diquota_nfs\n");
    }
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    strncpy (host, diqinfo->special, DI_SPEC_NAME_LEN);
    path = host;
    ptr = strchr (host, ':');
    if (ptr != (char *) NULL) {
      *ptr = '\0';
      path = ptr + 1;
    }
    if (debug > 2) {
      printf ("quota: nfs: host: %s path: %s\n", host, path);
    }
    args.gqa_pathp = path;
    args.gqa_uid = (int) diqinfo->uid;

    rqclnt = clnt_create (host, (unsigned long) RQUOTAPROG,
        (unsigned long) RQUOTAVERS, "udp");
    if (rqclnt == (CLIENT *) NULL) {
      if (debug > 2) {
        printf ("quota: nfs: create failed %d\n", errno);
      }
      return;
    }
    rqclnt->cl_auth = authunix_create_default();
    clnt_stat = clnt_call (rqclnt, (unsigned long) RQUOTAPROC_GETQUOTA,
        (xdrproc_t) xdr_quota_get, (caddr_t) &args,
        (xdrproc_t) xdr_quota_rslt, (caddr_t) &result, timeout);
    if (clnt_stat != RPC_SUCCESS) {
      if (debug > 2) {
        printf ("quota: nfs: not success\n");
      }
      if (rqclnt->cl_auth) {
        auth_destroy (rqclnt->cl_auth);
      }
      clnt_destroy (rqclnt);
      return;
    }

# if _mem_struct_getquota_rslt_gqr_status
    quotastat = result.gqr_status;
# else
    quotastat = result.status;
# endif
    if (quotastat == 1) {
# if _mem_struct_getquota_rslt_gqr_rquota
      rptr = &result.gqr_rquota;
# else
      rptr = &result.getquota_rslt_u.gqr_rquota;
# endif

      if (debug > 2) {
        printf ("quota: nfs: status 1\n");
        printf ("quota: nfs: rq_bsize: %d\n", rptr->rq_bsize);
        printf ("quota: nfs: rq_active: %d\n", rptr->rq_active);
      }

      tblksize = 512;

      diqinfo->limit = (_fs_size_t) rptr->rq_bhardlimit *
            (_fs_size_t) rptr->rq_bsize / tblksize;
      tsize = (_fs_size_t) rptr->rq_bsoftlimit *
              (_fs_size_t) rptr->rq_bsize / tblksize;
      if (tsize != 0 && tsize < diqinfo->limit) {
        diqinfo->limit = tsize;
      }
      if (diqinfo->limit != 0) {
        diqinfo->used = (_fs_size_t) rptr->rq_curblocks *
              (_fs_size_t) rptr->rq_bsize / tblksize;

        /* diqinfo->blockSize is sometimes too large to use */
        /* reset it to a low common denominator             */
        diqinfo->blockSize = tblksize;
      }

      diqinfo->ilimit = rptr->rq_fhardlimit;
      tsize = rptr->rq_fsoftlimit;
      if (tsize != 0 && tsize < diqinfo->ilimit) {
        diqinfo->ilimit = tsize;
      }
      if (diqinfo->ilimit != 0) {
        diqinfo->iused = rptr->rq_curfiles;
      }
    }

    if (rqclnt->cl_auth) {
      auth_destroy (rqclnt->cl_auth);
    }
    clnt_destroy (rqclnt);
}
#endif  /* have std nfs quotas */

#if _has_std_quotas
static void
# if _proto_stdc
di_process_quotas (char *tag, diQuota_t *diqinfo,
                  int rc, int xfsflag, qdata_t *qdata)
# else
di_process_quotas (tag, diqinfo, rc, xfsflag, qdata)
  char          *tag;
  diQuota_t     *diqinfo;
  int           rc;
  int           xfsflag;
  qdata_t       *qdata;
# endif
{
  _fs_size_t        quotBlockSize = { DI_QUOT_BLOCK_SIZE };
  _fs_size_t        tsize;
  _fs_size_t        tlimit;

  if (debug > 5) {
    printf ("quota: di_process_quotas\n");
  }
  if (xfsflag) {
    quotBlockSize = 512;
  }

  if (diqinfo->blockSize == 0) {
    diqinfo->blockSize = 512;  /* set it to something to prevent crashes */
  }

  if (rc == 0) {
    tlimit = 0;
# if _typ_fs_disk_quota_t
    tsize = qdata->xfsqinfo.d_blk_hardlimit;
# endif
# if _typ_struct_quotaval
    tsize = qdata->qval.qbval.qv_hardlimit;
# endif
# if _typ_struct_dqblk
    tsize = qdata->qinfo.dqb_bhardlimit;
# endif
# if _typ_struct_ufs_dqblk
    tsize = qdata->ufsqinfo.dqb_bhardlimit;
# endif
    if (debug > 2) {
      printf ("quota: %s %s b hard: %lld\n", tag, diqinfo->name, tsize);
    }

    if ((_s_fs_size_t) tsize > 0) {
      tsize = tsize * quotBlockSize / diqinfo->blockSize;
      if (tsize > 0 && (tsize < diqinfo->limit || diqinfo->limit == 0)) {
        if (debug > 2) {
          printf ("quota: using b hard: %lld\n", tsize);
        }
        diqinfo->limit = tsize;
        tlimit = tsize;
      }
    }

# if _typ_fs_disk_quota_t
    tsize = qdata->xfsqinfo.d_blk_softlimit;
# endif
# if _typ_struct_quotaval
    tsize = qdata->qval.qbval.qv_softlimit;
# endif
# if _typ_struct_dqblk
    tsize = qdata->qinfo.dqb_bsoftlimit;
# endif
# if _typ_struct_ufs_dqblk
    tsize = qdata->ufsqinfo.dqb_bsoftlimit;
# endif
    if (debug > 2) {
      printf ("quota: %s %s b soft: %lld\n", tag, diqinfo->name, tsize);
    }
    if ((_s_fs_size_t) tsize > 0) {
      tsize = tsize * quotBlockSize / diqinfo->blockSize;
      if (tsize > 0 && (tsize < diqinfo->limit || diqinfo->limit == 0)) {
        if (debug > 2) {
          printf ("quota: using b soft: %lld\n", tsize);
        }
        diqinfo->limit = tsize;
        tlimit = tsize;
      }
    }

      /* any quota set? */
    if (tlimit == 0) {
      if (debug > 2) {
        printf ("quota: %s %s no quota\n", tag, diqinfo->name);
      }
      return;
    }

# if _mem_struct_dqblk_dqb_curspace
    tsize = qdata->qinfo.dqb_curspace / diqinfo->blockSize;
    if (tsize > diqinfo->used || diqinfo->used == 0) {
      diqinfo->used = tsize;
    }
# endif
# if _typ_struct_quotaval
    tsize = qdata->qval.qbval.qv_usage;
# endif
# if _mem_struct_dqblk_dqb_curblocks
    tsize = qdata->qinfo.dqb_curblocks;
# endif
# if _typ_fs_disk_quota_t
    tsize = qdata->xfsqinfo.d_bcount;
# endif
    tsize = tsize * quotBlockSize / diqinfo->blockSize;
    if (tsize > diqinfo->used || diqinfo->used == 0) {
      diqinfo->used = tsize;
    }

    if (debug > 2) {
      printf ("quota: %s %s used: %lld limit: %lld\n", tag, diqinfo->name,
          diqinfo->used, diqinfo->limit);
    }

# if _typ_fs_disk_quota_t
    tsize = qdata->xfsqinfo.d_ino_hardlimit;
# endif
# if _typ_struct_quotaval
    tsize = qdata->qval.qival.qv_hardlimit;
# endif
# if _typ_struct_dqblk
    tsize = qdata->qinfo.dqb_ihardlimit;
# endif
# if _typ_struct_ufs_dqblk
    tsize = qdata->ufsqinfo.dqb_ihardlimit;
# endif
    if (tsize > 0 && (tsize < diqinfo->ilimit || diqinfo->ilimit == 0)) {
      diqinfo->ilimit = tsize;
      tlimit = tsize;
    }
    if (debug > 2) {
      printf ("quota: %s %s i hard: %lld\n", tag, diqinfo->name, tsize);
    }

# if _typ_fs_disk_quota_t
    tsize = qdata->xfsqinfo.d_ino_softlimit;
# endif
# if _typ_struct_quotaval
    tsize = qdata->qval.qival.qv_softlimit;
# endif
# if _typ_struct_dqblk
    tsize = qdata->qinfo.dqb_isoftlimit;
# endif
# if _typ_struct_ufs_dqblk
    tsize = qdata->ufsqinfo.dqb_isoftlimit;
# endif
    if (tsize > 0 && (tsize < diqinfo->ilimit || diqinfo->ilimit == 0)) {
      diqinfo->ilimit = tsize;
      tlimit = tsize;
    }
    if (debug > 2) {
      printf ("quota: %s %s i soft: %lld\n", tag, diqinfo->name, tsize);
    }

      /* any quota set? */
    if (tlimit == 0) {
      if (debug > 2) {
        printf ("quota: %s %s no inode quota\n", tag, diqinfo->name);
      }
      return;
    }

# if _typ_fs_disk_quota_t
    tsize = qdata->xfsqinfo.d_icount;
# endif
# if _typ_struct_quotaval
    tsize = qdata->qval.qival.qv_usage;
# endif
# if _mem_struct_dqblk_dqb_curinodes
    tsize = qdata->qinfo.dqb_curinodes
# endif
    if (tsize > diqinfo->iused || diqinfo->iused == 0) {
      diqinfo->iused = tsize;
    }
    if (debug > 2) {
      printf ("quota: %s %s i used: %lld\n", tag, diqinfo->name, tsize);
    }
  } else {
    if (debug > 2) {
      printf ("quota: %s %s errno %d\n", tag, diqinfo->name, errno);
    }
  }
}
#endif /* _has_std_quotas */
