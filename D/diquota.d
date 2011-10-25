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
  string        fsType;
  Uid_t         uid;
  Gid_t         gid;
  real          limit;
  real          used;
  real          ilimit;
  real          iused;
}


static if (_cdefine_BLOCK_SIZE) {
  alias BLOCK_SIZE DI_QUOT_BLOCK_SIZE;  /* linux */
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
    diqinfo.name = dp.name;
    diqinfo.special = dp.special;
    diqinfo.fsType = dp.fsType;

    diquota (&diqinfo, opts);

    if (opts.debugLevel > 2) {
      writefln ("quota: %s limit: %f", dp.name, diqinfo.limit);
      writefln ("quota:   tot: %f", dp.totalBlocks);
      writefln ("quota: %s used: %f", dp.name, diqinfo.used);
      writefln ("quota:   avail: %f", dp.availBlocks);
    }
    if (diqinfo.limit != 0 && diqinfo.limit < dp.totalBlocks) {
      dp.totalBlocks = diqinfo.limit;
      tsize = diqinfo.limit - diqinfo.used;
      if (tsize < 0) {
        tsize = 0;
      }
      if (tsize < dp.availBlocks) {
        dp.availBlocks = tsize;
        dp.freeBlocks = tsize;
        if (opts.debugLevel > 2) {
          writefln ("quota: using quota for: total free avail");
        }
      } else if (tsize > dp.availBlocks && tsize < dp.freeBlocks) {
        dp.freeBlocks = tsize;
        if (opts.debugLevel > 2) {
          writefln ("quota: using quota for: total free");
        }
      } else {
        if (opts.debugLevel > 2) {
          writefln ("quota: using quota for: total");
        }
      }
    }

    if (diqinfo.ilimit != 0 && diqinfo.ilimit < dp.totalInodes) {
      dp.totalInodes = diqinfo.ilimit;
      tsize = diqinfo.ilimit - diqinfo.iused;
      if (tsize < 0) {
        tsize = 0;
      }
      if (tsize < dp.availInodes) {
        dp.availInodes = tsize;
        dp.freeInodes = tsize;
        if (opts.debugLevel > 2) {
          writefln ("quota: using quota for inodes: total free avail");
        }
      } else if (tsize > dp.availInodes && tsize < dp.freeInodes) {
        dp.freeInodes = tsize;
        if (opts.debugLevel > 2) {
          writefln ("quota: using quota for inodes: total free");
        }
      } else {
        if (opts.debugLevel > 2) {
          writefln ("quota: using quota for inodes: total");
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
    writefln ("quota: diquota %s %s ", diqinfo.name, diqinfo.fsType);
  }
  rc = -1;
  xfsflag = FALSE;

  diqinfo.limit = 0;
  diqinfo.used = 0;
  diqinfo.ilimit = 0;
  diqinfo.iused = 0;

  if (diqinfo.fsType.length >= 3 && diqinfo.fsType[0..3] == "nfs" &&
      diqinfo.fsType != "nfsd") {
    if (opts.debugLevel > 5) {
      writefln ("quota: diquota: nfs");
    }
    static if (_has_std_nfs_quotas) {
      diquota_nfs (diqinfo, opts);
    }
    return;
  }
  if (diqinfo.fsType == "xfs") {
    if (opts.debugLevel > 5) {
      writefln ("quota: diquota: xfs");
    }
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

  static if (_cdefine___FreeBSD__ && _t_FreeBSD__ == 5) {
      /* quotactl on devfs fs panics the system (FreeBSD 5.1) */
    if (strcmp (diqinfo.fsType, "ufs") != 0) {
      return;
    }
  }

  if (opts.debugLevel > 5) {
    writefln ("quota: quotactl on %s (%s)", diqinfo.name, _c_arg_1_quotactl);
  }
  static if (_clib_quotactl && _c_arg_1_quotactl == "char *") {
    rc = quotactl (toStringz(diqinfo.name), ucmd,
          cast(int) diqinfo.uid, cast(caddr_t) qiptr);
  }
  static if (_clib_quotactl && _c_arg_2_quotactl == "char *") {
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

  if (opts.debugLevel > 5) {
    writefln ("quota: quotactl on %s : rc %d : errno %d", diqinfo.name, rc, getErrno);
  }
  static if (_has_std_quotas) {
    di_process_quotas ("usr", diqinfo, rc, xfsflag, qiptr, opts);

    static if (_cdefine_GRPQUOTA) {
      if (rc == 0 || getErrno != ESRCH) {
        static if (_clib_quotactl && _c_arg_1_quotactl == "char *") {
          rc = quotactl (toStringz(diqinfo.name), gcmd,
              cast(int) diqinfo.gid, cast(caddr_t) qiptr);
        }
        static if (_clib_quotactl && _c_arg_1_quotactl != "char *") {
          rc = quotactl (gcmd, toStringz(diqinfo.special),
              cast(int) diqinfo.gid, cast(caddr_t) qiptr);
        }

        di_process_quotas ("grp", diqinfo, rc, xfsflag, qiptr, opts);
      }
    } /* _cdefine_GRPQUOTA */
  } /* _has_std_quotas */
}

static if (_has_std_nfs_quotas) {

static if (_cdefine_RQ_PATHLEN) {
  alias RQ_PATHLEN DI_RQ_PATHLEN;
} else {
  enum int DI_RQ_PATHLEN = 1024;
}

extern (C) {

  static C_TYP_bool_t
  xdr_quota_get (XDR *xp, C_ST_getquota_args *args)
  {
    if (! xdr_string (xp, &args.gqa_pathp, DI_RQ_PATHLEN)) {
      return 0;
    }
    if (! xdr_gqa_uid (xp, &args.gqa_uid)) {
      return 0;
    }
    return 1;
  }

  static C_TYP_bool_t
  xdr_quota_rslt (XDR *xp, C_ST_getquota_rslt *rslt)
  {
    C_ENUM_gqr_status quotastat;
    int               intquotastat;
    C_ST_rquota       *rptr;

    if (! xdr_int (xp, &intquotastat)) {
      return 0;
    }
    quotastat = cast(C_ENUM_gqr_status) intquotastat;
    static if (_cmem_getquota_rslt_gqr_status) {
      rslt.gqr_status = quotastat;
    } else {
      rslt.status = quotastat;
    }
    rptr = &rslt.gqr_rquota;

    if (! xdr_int (xp, &rptr.rq_bsize)) {
      return 0;
    }
    if (! xdr_bool (xp, &rptr.rq_active)) {
      return 0;
    }
    if (! xdr_rq_bhardlimit (xp, &rptr.rq_bhardlimit)) {
      return 0;
    }
    if (! xdr_rq_bsoftlimit (xp, &rptr.rq_bsoftlimit)) {
      return 0;
    }
    if (! xdr_rq_curblocks (xp, &rptr.rq_curblocks)) {
      return 0;
    }
    if (! xdr_rq_fhardlimit (xp, &rptr.rq_fhardlimit)) {
      return 0;
    }
    if (! xdr_rq_fsoftlimit (xp, &rptr.rq_fsoftlimit)) {
      return 0;
    }
    if (! xdr_rq_curfiles (xp, &rptr.rq_curfiles)) {
      return 0;
    }
    return (1);
  }
} /* extern (C) */

static void
diquota_nfs (diQuota_t *diqinfo, Options opts)
{
    C_ST_CLIENT             *rqclnt;
    C_ENUM_clnt_stat        clnt_stat;
    C_ST_timeval            timeout;
    string                  host;
    string                  path;
    char[]                  hostz;
    char[]                  pathz;
    C_ST_getquota_args      args;
    C_ST_getquota_rslt      result;
    C_ST_rquota             *rptr;
    int                     quotastat;
    real                    tsize;

    if (opts.debugLevel > 5) {
      writefln ("quota: diquota_nfs");
    }
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    host = diqinfo.special;
    path = host;
    hostz = host.dup;
    auto idx = indexOf (host, ':');
    if (idx != -1) {
      hostz[idx] = '\0';
      ++idx;
      path = host[idx..$];
    }
    pathz = path.dup;
    pathz ~= '\0';
    if (opts.debugLevel > 5) {
      writefln ("quota: nfs: host: %s path: %s", host, path);
      writefln ("quota: nfs: hostz: %s pathz: %s", hostz, pathz);
    }
    args.gqa_pathp = pathz.ptr;
    args.gqa_uid = cast(C_TYP_gqa_uid) diqinfo.uid;

    rqclnt = clnt_create (hostz.ptr, cast(C_TYP_u_long) RQUOTAPROG,
        cast(C_TYP_u_long) RQUOTAVERS, toStringz("udp"));
    if (rqclnt == cast(CLIENT *) null) {
      if (opts.debugLevel > 2) {
        writefln ("quota: nfs: create failed %d", getErrno);
      }
      return;
    }
    rqclnt.cl_auth = authunix_create_default();
    clnt_stat = C_MACRO_clnt_call (rqclnt, cast(C_TYP_u_long) RQUOTAPROC_GETQUOTA,
        cast(xdrproc_t) &xdr_quota_get, cast(caddr_t) &args,
        cast(xdrproc_t) &xdr_quota_rslt, cast(caddr_t) &result, timeout);
    if (clnt_stat != C_ENUM_clnt_stat.RPC_SUCCESS) {
      if (opts.debugLevel > 2) {
        writefln ("quota: nfs: not success: %d", cast(int) clnt_stat);
      }
      if (rqclnt.cl_auth) {
        C_MACRO_auth_destroy (rqclnt.cl_auth);
      }
      C_MACRO_clnt_destroy (rqclnt);
      return;
    }

    static if (_cmem_getquota_rslt_gqr_status) {
      quotastat = result.gqr_status;
    } else {
      quotastat = result.status;
    }
    if (quotastat == 1) {
      rptr = &result.gqr_rquota;

      if (opts.debugLevel > 2) {
        writefln ("quota: nfs: status 1");
        writefln ("quota: nfs: rq_bsize: %d", rptr.rq_bsize);
        writefln ("quota: nfs: rq_active: %d", rptr.rq_active);
      }

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
      C_MACRO_auth_destroy (rqclnt.cl_auth);
    }
    C_MACRO_clnt_destroy (rqclnt);
}
} /* have rpc headers */

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
      writefln ("quota: %s %s orig hard: %f", tag, diqinfo.name, tsize);
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
      writefln ("quota: %s %s orig soft: %f", tag, diqinfo.name, tsize);
    }
// OLD:    tsize = tsize * quotBlockSize / diqinfo.blockSize;
    if (tsize != 0 && (tsize < diqinfo.limit || diqinfo.limit == 0)) {
      if (opts.debugLevel > 2) {
        writefln ("quota: using soft: %f", tsize);
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
      writefln ("quota: %s %s used: %f limit: %f", tag, diqinfo.name,
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
      writefln ("quota: %s %s i hard: %f", tag, diqinfo.name, tsize);
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
      writefln ("quota: %s %s i soft: %f", tag, diqinfo.name, tsize);
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
      writefln ("quota: %s %s i used: %f", tag, diqinfo.name, tsize);
    }
  } else {
    if (opts.debugLevel > 2) {
      writefln ("quota: %s %s errno %d", tag, diqinfo.name, getErrno);
    }
  }
}
} /* _has_std_quotas */
