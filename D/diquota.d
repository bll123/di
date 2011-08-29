// written in the D programming language

module diquota;

import std.stdio;
import std.string;
private import std.process : geteuid, getegid;
private import core.stdc.errno;

import config;
import options;
import diskpart;

struct diQuota_t {
  string        special;
  string        name;
  string        type;
  Uid_t         uid;
  Gid_t         gid;
  real          limit;
  real          used;
  real          ilimit;
  real          iused;
}


static if (_cdefine_BLOCK_SIZE) {
  alias BLOCK_SIZE DI_QUOT_BLOCK_SIZE;  /* linux *?
} else static if (_cdefine_DQBSIZE) {
  alias DQBSIZE DI_QUOT_BLOCK_SIZE;   /* aix */
} else static if (_cdefine_DEV_BSIZE) {
  alias DEV_BSIZE DI_QUOT_BLOCK_SIZE;   /* tru64, et. al. */
} else {
  enum size_t DI_QUOT_BLOCK_SIZE = 512;
}

  /* rename certain structure members for portability */
static if (_cmem_dqblk_dqb_fsoftlimit) {
  alias dqb_fsoftlimit dqb_isoftlimit;
}
static if (_cmem_dqblk_dqb_fhardlimit) {
  alias dqb_fhardlimit dqb_ihardlimit;
}
static if (_cmem_dqblk_dqb_curfiles) {
  alias dqb_curfiles dqb_curinodes;
}

void
checkDiskQuotas (ref DiskPartitions dpList, Options opts)
{
  int           i;
  Uid_t         uid;
  Gid_t         gid;
  diQuota_t     diqinfo;
  real          tsize;

  uid = 0;
  gid = 0;
static if (_has_std_quotas) {
  uid = geteuid ();
  gid = getegid ();
}

  foreach (ref dp; dpList.diskPartitions)
  {
//   if (! dp.doPrint) {
//      continue;
//    }

    diqinfo.uid = uid;
    diqinfo.gid = gid;
    diquota (&diqinfo, opts);

//    if (opts.debugLevel > 2) {
//      writefln ("quota: %s limit: %d", dinfo.name, diqinfo.limit);
//      writefln ("quota:   tot: %d", dinfo.totalBlocks);
//      writefln ("quota: %s used: %d", dinfo.name, diqinfo.used);
//      writefln ("quota:   avail: %d", dinfo.availBlocks);
//    }
    if (diqinfo.limit != 0 &&
            diqinfo.limit < dp.totalBlocks) {
      dp.totalBlocks = diqinfo.limit;
      tsize = diqinfo.limit - diqinfo.used;
      if (tsize < 0) {
        tsize = 0;
      }
      if (tsize < dp.availBlocks) {
        dp.availBlocks = tsize;
        dp.freeBlocks = tsize;
        if (opts.debugLevel > 2) {
          printf ("quota: using quota for: total free avail\n");
        }
      } else if (tsize > dp.availBlocks && tsize < dp.freeBlocks) {
        dp.freeBlocks = tsize;
        if (opts.debugLevel > 2) {
          printf ("quota: using quota for: total free\n");
        }
      } else {
        if (opts.debugLevel > 2) {
          printf ("quota: using quota for: total\n");
        }
      }
    }

    if (diqinfo.ilimit != 0 &&
            diqinfo.ilimit < dp.totalInodes) {
      dp.totalInodes = diqinfo.ilimit;
      tsize = diqinfo.ilimit - diqinfo.iused;
      if (tsize < 0) {
        tsize = 0;
      }
      if (tsize < dp.availInodes) {
        dp.availInodes = tsize;
        dp.freeInodes = tsize;
        if (opts.debugLevel > 2) {
          printf ("quota: using quota for inodes: total free avail\n");
        }
      } else if (tsize > dp.availInodes && tsize < dp.freeInodes) {
        dp.freeInodes = tsize;
        if (opts.debugLevel > 2) {
          printf ("quota: using quota for inodes: total free\n");
        }
      } else {
        if (opts.debugLevel > 2) {
          printf ("quota: using quota for inodes: total\n");
        }
      }
    }
  }
  return;
}

void
diquota (diQuota_t *diqinfo, Options opts)
{
  int               rc;
  int               xfsflag;
static if (_cstruct_dqblk) {
  C_ST_dqblk       qinfo;
  char             *qiptr;
}
static if (_cstruct_fs_disk_quota) {
  C_ST_fs_disk_quota   xfsqinfo;
}
static if (_has_std_quotas) {
  int               ucmd;
  int               gcmd;
}

  if (opts.debugLevel > 5) {
    writefln ("quota: diquota");
  }
  rc = -1;
  xfsflag = FALSE;

  diqinfo.limit = 0;
  diqinfo.used = 0;
  diqinfo.ilimit = 0;
  diqinfo.iused = 0;

  if (diqinfo.type[0..2] == "nfs" &&
      diqinfo.type != "nfsd") {
static if (_hdr_rpc_rpc && _hdr_rpcsvc_rquota) {
    //diquota_nfs (diqinfo);
}
    return;
  }
  if (diqinfo.type == "xfs") {
static if (_hdr_linux_dqblk_xfs) {
    ucmd = C_MACRO_QCMD (Q_XGETQUOTA, USRQUOTA);
    gcmd = C_MACRO_QCMD (Q_XGETQUOTA, GRPQUOTA);
    qiptr = cast(char *) &xfsqinfo;
    xfsflag = TRUE;
}
    ;
  } else {
static if (_has_std_quotas) {
    /* hp-ux doesn't have QCMD */
    ucmd = Q_GETQUOTA;
    gcmd = Q_GETQUOTA;
}
static if (_cmacro_QCMD) {
    ucmd = C_MACRO_QCMD (Q_GETQUOTA, USRQUOTA);
    gcmd = C_MACRO_QCMD (Q_GETQUOTA, GRPQUOTA);
}
static if (_cstruct_dqblk) {
    qiptr = cast(char *) &qinfo;
}
  }

static if (_cdefine___FreeBSD__ == 5) {
    /* quotactl on devfs fs panics the system (FreeBSD 5.1) */
  if (strcmp (diqinfo.type, "ufs") != 0) {
    return;
  }
}

  if (opts.debugLevel > 5) {
    writefln ("quota: quotactl on %s (%s)", diqinfo.name, _c_arg_1_quotactl);
  }
static if (_clib_quotactl && _c_arg_1_quotactl == "char") {
  rc = quotactl (toStringz(diqinfo.name), ucmd,
        cast(int) diqinfo.uid, cast(caddr_t) qiptr);
}
static if (_clib_quotactl && _c_arg_1_quotactl != "char") {
 /* AIX has linux compatibility routine, but still need name */
static if (_cdefine__AIX) {
  rc = quotactl (ucmd, toStringz(diqinfo.name),
        cast(int) diqinfo.uid, cast(caddr_t) qiptr);
} else {
  rc = quotactl (ucmd, toStringz(diqinfo.special),
        cast(int) diqinfo.uid, cast(caddr_t) qiptr);
}
}
static if (_sys_fs_ufs_quota) {       /* Solaris */
  {
    int             fd;
    C_ST_quotctl    qop;
    char            tname [DI_NAME_LEN];

    qop.op = Q_GETQUOTA;
    qop.uid = diqinfo.uid;
    qop.addr = cast(caddr_t) qiptr;
    strcpy (tname, diqinfo.name);
    strcat (tname, "/quotas");
    fd = open (tname, O_RDONLY | O_NOCTTY);
    if (fd >= 0) {
      rc = ioctl (fd, Q_QUOTACTL, &qop);
      close (fd);
    } else {
      rc = fd;
    }
  }
} /* _sys_fs_ufs_quota */

static if (_has_std_quotas) {
  di_process_quotas ("usr", diqinfo, rc, xfsflag, qiptr, opts);

static if (_cdefine_GRPQUOTA) {
  if (rc == 0 || getErrno != ESRCH) {
static if (_clib_quotactl && _c_arg_1_quotactl == "char") {
    rc = quotactl (toStringz(diqinfo.name), gcmd,
          cast(int) diqinfo.gid, cast(caddr_t) qiptr);
}
static if (_clib_quotactl && _c_arg_1_quotactl != "char") {
    rc = quotactl (gcmd, toStringz(diqinfo.special),
             cast(int) diqinfo.gid, cast(caddr_t) qiptr);
}

    di_process_quotas ("grp", diqinfo, rc, xfsflag, qiptr, opts);
  }
} /* _cdefine_GRPQUOTA */
} /* _has_std_quotas */
}

/+
static if (_hdr_rpc_rpc && _hdr_rpcsvc_rquota) {

static if (_cdefine_RQ_PATHLEN) {
  alias RQ_PATHLEN DI_RQ_PATHLEN;
} else {
  enum int DI_RQ_PATHLEN = 1024;
}

static C_TYP_bool_t
xdr_quota_get (XDR *xp, C_ST_getquota_args *args)
{
  if (opts.debugLevel > 5) {
    writefln ("quota: xdr_quota_get");
  }

  if (! xdr_string (xp, &args.gqa_pathp, DI_RQ_PATHLEN)) {
    return 0;
  }
  if (! _gqa_uid_xdr (xp, &args.gqa_uid)) {
    return 0;
  }
  return 1;
}

static C_TYP_bool_t
xdr_quota_rslt (XDR *xp, C_ST_getquota_rslt *rslt)
{
  int           quotastat;
  C_ST_rquota   *rptr;

  if (opts.debugLevel > 5) {
    writefln ("quota: xdr_quota_rslt");
  }

  if (! xdr_int (xp, &quotastat)) {
    return 0;
  }
static if (_cmem_getquota_rslt_gqr_status) {
  rslt.gqr_status = quotastat;
} else {
  rslt.status = quotastat;
}
static if (_cmem_getquota_rslt_gqr_rquota) {
  rptr = &rslt.gqr_rquota;
} else {
  rptr = &rslt.getquota_rslt_u.gqr_rquota;
}

  if (! xdr_int (xp, &rptr.rq_bsize)) {
    return 0;
  }
  if (! xdr_bool (xp, &rptr.rq_active)) {
    return 0;
  }
  if (! _rquota_xdr (xp, &rptr.rq_bhardlimit)) {
    return 0;
  }
  if (! _rquota_xdr (xp, &rptr.rq_bsoftlimit)) {
    return 0;
  }
  if (! _rquota_xdr (xp, &rptr.rq_curblocks)) {
    return 0;
  }
  if (! _rquota_xdr (xp, &rptr.rq_fhardlimit)) {
    return 0;
  }
  if (! _rquota_xdr (xp, &rptr.rq_fsoftlimit)) {
    return 0;
  }
  if (! _rquota_xdr (xp, &rptr.rq_curfiles)) {
    return 0;
  }
  return (1);
}

static void
diquota_nfs (diQuota_t *diqinfo)
{
    C_ST_CLIENT             *rqclnt;
    enum C_ENUM_clnt_stat   clnt_stat;
    C_ST_timeval            timeout;
    char                    host [DI_SPEC_NAME_LEN];
    char                    *ptr;
    char                    *path;
    C_ST_getquota_args      args;
    C_ST_getquota_rslt      result;
    C_ST_rquota             *rptr;
    int                     quotastat;
    _fs_size_t              tsize;

    if (opts.debugLevel > 5) {
      writefln ("quota: diquota_nfs");
    }
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    strcpy (host, diqinfo.special);
    path = host;
    ptr = strchr (host, ':');
    if (ptr != cast(char *) NULL) {
      *ptr = '\0';
      path = ptr + 1;
    }
    if (opts.debugLevel > 2) {
      writefln ("quota: nfs: host: %s path: %s", host, path);
    }
    args.gqa_pathp = path;
    args.gqa_uid = cast(int) diqinfo.uid;

    rqclnt = clnt_create (host, cast(ulong) RQUOTAPROG,
        cast(ulong) RQUOTAVERS, "udp");
    if (rqclnt == cast(CLIENT *) NULL) {
      if (opts.debugLevel > 2) {
        writefln ("quota: nfs: create failed %d", getErrno);
      }
      return;
    }
    rqclnt.cl_auth = authunix_create_default();
    clnt_stat = clnt_call (rqclnt, cast(ulong) RQUOTAPROC_GETQUOTA,
        cast(xdrproc_t) xdr_quota_get, cast(caddr_t) &args,
        cast(xdrproc_t) xdr_quota_rslt, cast(caddr_t) &result, timeout);
    if (clnt_stat != RPC_SUCCESS) {
      if (opts.debugLevel > 2) {
        writefln ("quota: nfs: not success");
      }
      if (rqclnt.cl_auth) {
        auth_destroy (rqclnt.cl_auth);
      }
      clnt_destroy (rqclnt);
      return;
    }

static if (_cmem_getquota_rslt_gqr_status) {
    quotastat = result.gqr_status;
} else {
    quotastat = result.status;
}
    if (quotastat == 1) {
static if (_cmem_getquota_rslt_gqr_rquota) {
      rptr = &result.gqr_rquota;
} else {
      rptr = &result.getquota_rslt_u.gqr_rquota;
}

      if (opts.debugLevel > 2) {
        writefln ("quota: nfs: status 1");
        writefln ("quota: nfs: rq_bsize: %d", rptr.rq_bsize);
        writefln ("quota: nfs: rq_active: %d", rptr.rq_active);
      }

      tblksize = 512;

      diqinfo.limit = rptr.rq_bhardlimit * rptr.rq_bsize;
      tsize = rptr.rq_bsoftlimit * rptr.rq_bsize;
      if (tsize != 0 && tsize < diqinfo.limit) {
        diqinfo.limit = tsize;
      }
      if (diqinfo.limit != 0) {
        diqinfo.used = rptr.rq_curblocks * rptr.rq_bsize;
      }

      diqinfo.ilimit = rptr.rq_fhardlimit;
      tsize = rptr.rq_fsoftlimit;
      if (tsize != 0 && tsize < diqinfo.ilimit) {
        diqinfo.ilimit = tsize;
      }
      if (diqinfo.ilimit != 0) {
        diqinfo.iused = rptr.rq_curfiles;
      }
    }

    if (rqclnt.cl_auth) {
      auth_destroy (rqclnt.cl_auth);
    }
    clnt_destroy (rqclnt);
}
} /* have rpc headers */
+/

static if (_has_std_quotas) {
static void
di_process_quotas (string tag, diQuota_t *diqinfo,
                  int rc, int xfsflag, char *cqinfo, Options opts)
{
  real              quotBlockSize = DI_QUOT_BLOCK_SIZE;
  real              tsize;
  real              tlimit;
  C_ST_dqblk        *qinfo;
static if (_cstruct_fs_disk_quota) {
  C_ST_fs_disk_quota   *xfsqinfo;
}

  if (opts.debugLevel > 5) {
    writefln ("quota: di_process_quotas");
  }
  qinfo = cast(C_ST_dqblk *) cqinfo;
  if (xfsflag) {
static if (_cstruct_fs_disk_quota) {
    xfsqinfo = cast(C_ST_fs_disk_quota *) cqinfo;
}
    quotBlockSize = 512;
  }

  if (rc == 0) {
    tlimit = 0;
    if (xfsflag) {
static if (_cstruct_fs_disk_quota) {
      tsize = xfsqinfo.d_blk_hardlimit;
}
      ;
    } else {
      tsize = qinfo.dqb_bhardlimit;
    }
    if (opts.debugLevel > 2) {
      writefln ("quota: %s %s orig hard: %lld", tag, diqinfo.name, tsize);
    }
// OLD:    tsize = tsize * quotBlockSize / diqinfo.blockSize;
    if (tsize != 0 && (tsize < diqinfo.limit || diqinfo.limit == 0)) {
      diqinfo.limit = tsize;
      tlimit = tsize;
    }

    if (xfsflag) {
static if (_cstruct_fs_disk_quota) {
      tsize = xfsqinfo.d_blk_softlimit;
}
      ;
    } else {
      tsize = qinfo.dqb_bsoftlimit;
    }
    if (opts.debugLevel > 2) {
      writefln ("quota: %s %s orig soft: %lld", tag, diqinfo.name, tsize);
    }
// OLD:    tsize = tsize * quotBlockSize / diqinfo.blockSize;
    if (tsize != 0 && (tsize < diqinfo.limit || diqinfo.limit == 0)) {
      if (opts.debugLevel > 2) {
        writefln ("quota: using soft: %lld", tsize);
      }
      diqinfo.limit = tsize;
      tlimit = tsize;
    }

      /* any quota set? */
    if (tlimit == 0) {
      if (opts.debugLevel > 2) {
        writefln ("quota: %s %s no quota", tag, diqinfo.name);
      }
      return;
    }

static if (_cmem_dqblk_dqb_curspace) {
// OLD:    tsize = qinfo.dqb_curspace / diqinfo.blockSize;
    if (tsize > diqinfo.used || diqinfo.used == 0) {
      diqinfo.used = tsize;
    }
}
static if (_cmem_dqblk_dqb_curblocks) {
    if (xfsflag) {
static if (_cstruct_fs_disk_quota) {
      tsize = xfsqinfo.d_bcount;
}
      ;
    } else {
      tsize = qinfo.dqb_curblocks;
    }
// OLD:    tsize = tsize * quotBlockSize / diqinfo.blockSize;
    if (tsize > diqinfo.used || diqinfo.used == 0) {
      diqinfo.used = tsize;
    }
}

    if (opts.debugLevel > 2) {
      writefln ("quota: %s %s used: %lld limit: %lld", tag, diqinfo.name,
          diqinfo.used, diqinfo.limit);
    }

    if (xfsflag) {
static if (_cstruct_fs_disk_quota) {
      tsize = xfsqinfo.d_ino_hardlimit;
}
      ;
    } else {
      tsize = qinfo.dqb_ihardlimit;
    }
    if (tsize != 0 && (tsize < diqinfo.ilimit || diqinfo.ilimit == 0)) {
      diqinfo.ilimit = tsize;
      tlimit = tsize;
    }
    if (opts.debugLevel > 2) {
      writefln ("quota: %s %s i hard: %lld", tag, diqinfo.name, tsize);
    }

    if (xfsflag) {
static if (_cstruct_fs_disk_quota) {
      tsize = xfsqinfo.d_ino_softlimit;
}
      ;
    } else {
      tsize = qinfo.dqb_isoftlimit;
    }
    if (tsize != 0 && (tsize < diqinfo.ilimit || diqinfo.ilimit == 0)) {
      diqinfo.ilimit = tsize;
      tlimit = tsize;
    }
    if (opts.debugLevel > 2) {
      writefln ("quota: %s %s i soft: %lld", tag, diqinfo.name, tsize);
    }

      /* any quota set? */
    if (tlimit == 0) {
      if (opts.debugLevel > 2) {
        writefln ("quota: %s %s no inode quota", tag, diqinfo.name);
      }
      return;
    }

    if (xfsflag) {
static if (_cstruct_fs_disk_quota) {
      tsize = xfsqinfo.d_icount;
}
      ;
    } else {
      tsize = qinfo.dqb_curinodes;
    }
    if (tsize > diqinfo.iused || diqinfo.iused == 0) {
      diqinfo.iused = tsize;
    }
    if (opts.debugLevel > 2) {
      writefln ("quota: %s %s i used: %lld", tag, diqinfo.name, tsize);
    }
  } else {
    if (opts.debugLevel > 2) {
      writefln ("quota: %s %s errno %d", tag, diqinfo.name, getErrno);
    }
  }
}
} /* _has_std_quotas */
