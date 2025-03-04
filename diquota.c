/*
 * Copyright 2011-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#include "config.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stddef
# include <stddef.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_stdbool
# include <stdbool.h>
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
#if _sys_types \
    && ! defined (DI_INC_SYS_TYPES_H) /* xenix */
# define DI_INC_SYS_TYPES_H
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
#if _hdr_libprop_proplib        /* dragonflybsd */
# include <libprop/proplib.h>
#endif
#if _hdr_quota
# include <quota.h>
#endif
#if _sys_quota                  /* netbsd */
# include <sys/quota.h>
#endif
#if _sys_fs_ufs_quota
# include <sys/fs/ufs_quota.h>
#endif
#if _sys_vfs_quota              /* dragonflybsd */
# include <sys/vfs_quota.h>
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
// ### FIX
/* AIX 5.1 doesn't seem to have quotactl declared.... */
/* use their compatibility routine.                   */
#if ! _args_quotactl && _hdr_linux_quota \
      && _inc_conflict__sys_quota__hdr_linux_quota
# include <linux/quota.h>
#endif
#if _hdr_rpc_rpc
/* tirpc is a separate library, but defines reserved symbols */
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunknown-warning-option"
# pragma clang diagnostic ignored "-Wreserved-identifier"
# include <rpc/rpc.h>
# pragma clang diagnostic pop
#endif
#if _hdr_rpc_auth
# include <rpc/auth.h>
#endif
#if _hdr_rpcsvc_rquota
# include <rpcsvc/rquota.h>
#endif

#include "di.h"
#include "disystem.h"
#include "dimath.h"
#include "diquota.h"
#include "diinternal.h"
#include "distrutils.h"
#include "dioptions.h"

#if _has_std_quotas

/* workaround for HPUX - quotactl not declared */
# if _lib_quotactl && _npt_quotactl
#  if defined (__cplusplus) || defined (c_plusplus)
    extern "C" {
#  endif
  extern int quotactl (int, const char *, uid_t, void *);
#  if defined (__cplusplus) || defined (c_plusplus)
    }
#  endif
# endif

typedef union {
  int               val;
# if _typ_struct_dqblk
  struct dqblk      qinfo;
# endif
# if _typ_struct_ufs_dqblk
  struct ufs_dqblk  qinfo;
# endif
# if _typ_fs_disk_quota_t
  fs_disk_quota_t   xfsqinfo;
# endif
# if _typ_struct_quotaval
  struct qval {
    struct quotaval   qbval;
    struct quotaval   qival;
  } qval;
# endif
# if _lib_vquotactl
  struct vqval {
    struct vqvalentry {
      dinum_t      usage;
      dinum_t      limit;
    } uvqval;
    struct vqvalentry gvqval;
    struct vqvalentry *vqvalptr;
  } vqval;
# endif
} qdata_t;

static void di_process_quotas (di_data_t *di_data, const char *, di_quota_t *, int, int, qdata_t *);
#endif

#if _has_std_quotas && _lib_vquotactl
static void vq_updUsage (struct vqvalentry *, dinum_t);
static void vq_updLimit (struct vqvalentry *, dinum_t);
static int  vquotactl_send (char *, char *, prop_dictionary_t, prop_dictionary_t *);
static int  vquotactl_get (di_quota_t *, struct vqval *);
#endif
#if _has_std_quotas && _lib_quota_open
static int quota_open_get (struct quotahandle *, int, Uid_t, struct qval *);
#endif
#if _has_std_quotas && ! _lib_quota_open && ! _lib_vquotactl
static int quotactl_get (di_data_t *di_data, di_quota_t *, int, Uid_t, qdata_t *);
#endif
#if _has_std_nfs_quotas && ! _lib_quota_open
static bool_t xdr_quota_get (XDR *, struct getquota_args *);
static bool_t xdr_quota_rslt (XDR *, struct getquota_rslt *);
static void diquota_nfs (di_data_t *, di_quota_t *);
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

  /* dragonflybsd has a rather complicated method of getting    */
  /* the quota data.                                            */
  /* Since all results are returned, make sure to only iterate  */
  /* once.  Much of this code is straight from vquota.c.        */
#if _lib_vquotactl

static void
vq_updUsage (struct vqvalentry *entry, dinum_t usage)
{
  if (usage > entry->usage) {
    entry->usage = usage;
  }
}

static void
vq_updLimit (struct vqvalentry *entry, dinum_t limit)
{
  if (entry->values [DI_QUOTA_LIMIT] == 0 || limit < entry->values [DI_QUOTA_LIMIT]) {
    entry->values [DI_QUOTA_LIMIT] = limit;
  }
}

static int
vquotactl_send (char *spec, char *cmd,
    prop_dictionary_t args, prop_dictionary_t *res)
{
  prop_dictionary_t dict;
  struct plistref   pref;
  int               rv;
  int               error;

  dict = prop_dictionary_create ();

  if (dict == NULL) {
    return false;
  }

  rv = prop_dictionary_set_cstring (dict, "command", cmd);
  if (! rv) {
    prop_object_release (dict);
    return false;
  }

  rv = prop_dictionary_set (dict, "arguments", args);
  if (! rv) {
    prop_object_release (dict);
    return false;
  }

  error = prop_dictionary_send_syscall (dict, &pref);
  if (error != 0) {
    prop_object_release (dict);
    return false;
  }

  error = vquotactl (spec, &pref);
  if (error != 0) {
    prop_object_release (dict);
    return false;
  }

  error = prop_dictionary_recv_syscall (&pref, res);
  if (error != 0) {
    prop_object_release (dict);
    return false;
  }

  prop_object_release (dict);
  return true;
}

static int
vquotactl_get (di_quota_t *diqinfo, struct vqval *vqval)
{
  prop_dictionary_t         args;
  prop_dictionary_t         res;
  prop_array_t              reslist;
  prop_object_iterator_t    iter;
  prop_dictionary_t         item;
  int                       rv;
  int                       urv;
  int                       grv;
  Uid_t                     tuid;
  Uid_t                     tgid;
  dinum_t                   space;
  dinum_t                   limit;

  args = prop_dictionary_create ();
  if (args == NULL) { return errno; }
  res  = prop_dictionary_create ();
  if (res == NULL) { return errno; }

  rv = vquotactl_send (diqinfo->mountpt, "get usage all", args, &res);
  if (! rv) {
    prop_object_release (args);
    prop_object_release (res);
    return -4;
  }

  reslist = prop_dictionary_get (res, "returned data");
  if (reslist == NULL) {
    prop_object_release (args);
    prop_object_release (res);
    return errno;
  }

  iter = prop_array_iterator (reslist);
  if (iter == NULL) {
    prop_object_release (args);
    prop_object_release (res);
    return errno;
  }

  vqval->uvqval.usage = 0;
  vqval->uvqval.values [DI_QUOTA_LIMIT] = 0;
  vqval->gvqval.usage = 0;
  vqval->gvqval.values [DI_QUOTA_LIMIT] = 0;
  while ( (item = prop_object_iterator_next (iter)) != NULL) {
    rv = prop_dictionary_get_uint64 (item, "limit", &limit);
    if (rv && limit != 0) {
      rv = prop_dictionary_get_uint64 (item, "space used", &space);
      urv = prop_dictionary_get_uint32 (item, "uid", &tuid);
      grv = prop_dictionary_get_uint32 (item, "gid", &tgid);
      if (urv && tuid == diqinfo->uid) {
        vq_updUsage (& (vqval->uvqval), space);
        vq_updLimit (& (vqval->uvqval), limit);
      } else if (grv && tgid == diqinfo->gid) {
        vq_updUsage (& (vqval->gvqval), space);
        vq_updLimit (& (vqval->gvqval), limit);
      } else if (! urv && ! grv) {
        vq_updUsage (& (vqval->uvqval), space);
        vq_updLimit (& (vqval->uvqval), limit);
        vq_updUsage (& (vqval->gvqval), space);
        vq_updLimit (& (vqval->gvqval), limit);
      }
    }
  }
  prop_object_iterator_release (iter);
  prop_object_release (args);
  prop_object_release (res);
  return 0;
}
#endif  /* _lib_vquotactl */

#if _lib_quota_open
static int
quota_open_get (struct quotahandle *qh, int idtype,
                    Uid_t id, struct qval *qval)
{
  struct quotakey       qkey;
  int                   rc;

  rc = -3;
  if (qh != (struct quotahandle *) NULL) {
    memset (&qkey, 0, sizeof (struct quotakey));
    qkey.qk_idtype = idtype;
    qkey.qk_id = (id_t) id;
    qkey.qk_objtype = QUOTA_OBJTYPE_BLOCKS;
    rc = quota_get (qh, &qkey, & (qval->qbval));
    if (rc == 0) {
      qkey.qk_objtype = QUOTA_OBJTYPE_FILES;
      rc = quota_get (qh, &qkey, & (qval->qival));
    }
  }
  return rc;
}
#endif

#if _has_std_quotas && ! _lib_quota_open && ! _lib_vquotactl
static int
quotactl_get (di_data_t *di_data, di_quota_t *diqinfo, int cmd,
    Uid_t id, qdata_t *qdata)
{
  int       rc;
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  rc = -4;
# if defined (__FreeBSD__) && __FreeBSD__ == 5
  /* quotactl on devfs fs panics the system (FreeBSD 5.1) */
  if (strcmp (diqinfo->fstype, "ufs") != 0) {
    return -4;
  }
# endif

  if (diopts->optval [DI_OPT_DEBUG] > 5) {
    printf ("quota: quotactl on %s (%d %d)\n", diqinfo->mountpt,
            _quotactl_pos_1, _quotactl_pos_2);
  }
  /* AIX 7 has quotactl position 1 */
# if _lib_quotactl && _quotactl_pos_1
  rc = quotactl (diqinfo->mountpt, cmd, (int) id, (caddr_t) & (qdata->qinfo));
# endif
# if _lib_quotactl && ! _quotactl_pos_1 && (_quotactl_pos_2 || defined (_AIX))
#  if defined (_AIX)
  /* AIX has linux compatibility routine, */
  /* but needs mount-pt rather than dev-name */
  rc = quotactl (cmd, diqinfo->mountpt, (int) id, (caddr_t) & (qdata->qinfo));
#  else
  rc = quotactl (cmd, (_c_arg_2_quotactl) diqinfo->filesystem, (int) id, (caddr_t) & (qdata->qinfo));
#  endif
# endif
# if _has_std_quotas && _sys_fs_ufs_quota && ! _lib_vquotactl /* Solaris */
  {
    int             fd;
    struct quotctl  qop;
    char            tname [DI_MOUNTPT_LEN];
    char            *p;

    qop.op = Q_GETQUOTA;
    qop.uid = id;
    qop.addr = (caddr_t) & (qdata->qinfo);
    p = stpecpy (tname, tname + DI_MOUNTPT_LEN, diqinfo->mountpt);
    stpecpy (p, tname + DI_MOUNTPT_LEN, "/quotas");
    fd = open (tname, O_RDONLY | O_NOCTTY);
    if (fd >= 0) {
      rc = ioctl (fd, Q_QUOTACTL, &qop);
      close (fd);
    } else {
      rc = fd;
    }
  }
# endif  /* _sys_fs_ufs_quota */

  return rc;
}
#endif /* ! _lib_quota_open */

void
diquota (di_data_t *di_data, di_quota_t *diqinfo)
{
  int               rc;
  int               xfsflag;
#if _has_std_quotas
  qdata_t           qdata;
#endif
#if _lib_quota_open
  struct quotahandle    *qh;
#endif
#if _has_std_quotas && ! _lib_quota_open && ! _lib_vquotactl
  int               ucmd = 0;
  int               gcmd = 0;
#endif
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  rc = -1;
  xfsflag = false;

  dinum_set_u (&diqinfo->values [DI_QUOTA_LIMIT], (di_ui_t) 0);
  dinum_set_u (&diqinfo->values [DI_QUOTA_USED], (di_ui_t) 0);
  dinum_set_u (&diqinfo->values [DI_QUOTA_ILIMIT], (di_ui_t) 0);
  dinum_set_u (&diqinfo->values [DI_QUOTA_IUSED], (di_ui_t) 0);

#if _lib_vquotactl
  rc = vquotactl_get (diqinfo, &qdata.vqval);
  qdata.vqval.vqvalptr = &qdata.vqval.uvqval;
#endif

#if _lib_quota_open
  qh = quota_open (diqinfo->mountpt);
  rc = quota_open_get (qh, QUOTA_IDTYPE_USER, diqinfo->uid, &qdata.qval);
#endif

#if ! _lib_quota_open
  if (strncmp (diqinfo->fstype, "nfs", (Size_t) 3) == 0 &&
      strcmp (diqinfo->fstype, "nfsd") != 0) {
# if _has_std_nfs_quotas
    diquota_nfs (di_data, diqinfo);
# endif
    return;
  }
#endif

#if _has_std_quotas && ! _lib_quota_open && ! _lib_vquotactl
  if (strcmp (diqinfo->fstype, "xfs") == 0) {
# if _hdr_linux_dqblk_xfs
    ucmd = QCMD (Q_XGETQUOTA, USRQUOTA);
    gcmd = QCMD (Q_XGETQUOTA, GRPQUOTA);
    xfsflag = true;
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

  rc = quotactl_get (di_data, diqinfo, ucmd, diqinfo->uid, &qdata);
#endif /* _has_std_quotas && ! _lib_quota_open && ! _lib_vquotactl */

#if _has_std_quotas
  di_process_quotas (di_data, "usr", diqinfo, rc, xfsflag, &qdata);

#if _lib_vquotactl
  qdata.vqval.vqvalptr = &qdata.vqval.gvqval;
#endif
# if _lib_quota_open
  rc = quota_open_get (qh, QUOTA_IDTYPE_GROUP, diqinfo->uid, &qdata.qval);
# endif
# if ! _lib_quota_open && ! _lib_vquotactl
#  ifdef GRPQUOTA
  if (rc == 0 || errno != ESRCH) {
    rc = quotactl_get (di_data, diqinfo, gcmd, diqinfo->gid, &qdata);
  }
#  endif /* ifdef GRPQUOTA */
# endif /* ! _lib_quota_open && ! _lib_vquotactl */

# if _lib_quota_open
  if (qh != (struct quotahandle *) NULL) {
    quota_close (qh);
  }
# endif

# if defined (GRPQUOTA) || _lib_quota_open || _lib_vquotactl
  di_process_quotas (di_data, "grp", diqinfo, rc, xfsflag, &qdata);
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
xdr_quota_get (XDR *xp, struct getquota_args *args)
{
  if (! xdr_string (xp, &args->gqa_pathp, DI_RQ_PATHLEN)) {
    return 0;
  }
  if (! xdr_gqa_uid (xp, &args->gqa_uid)) {
    return 0;
  }
  return 1;
}

static bool_t
xdr_quota_rslt (XDR *xp, struct getquota_rslt *rslt)
{
  int           quotastat;
  struct rquota *rptr;

  if (! xdr_int (xp, &quotastat)) {
    return 0;
  }
# if _mem_struct_getquota_rslt_gqr_status
  rslt->gqr_status = quotastat;
# else
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wsign-conversion"
  rslt->status = (gqr_status) quotastat;
#  pragma clang diagnostic pop
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
diquota_nfs (di_data_t *di_data, di_quota_t *diqinfo)
{
  CLIENT                  *rqclnt;
  enum clnt_stat          clnt_stat;
  struct timeval          timeout;
  char                    host [DI_FILESYSTEM_LEN];
  char                    *ptr;
  char                    *path;
  struct getquota_args    args;
  struct getquota_rslt    result;
  struct rquota           *rptr;
  int                     quotastat;
  dinum_t                 tsize;
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  dinum_init (&tsize);

  if (diopts->optval [DI_OPT_DEBUG] > 5) {
    printf ("quota: diquota_nfs\n");
  }
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;

  stpecpy (host, host + DI_FILESYSTEM_LEN, diqinfo->filesystem);
  path = host;
  ptr = strchr (host, ':');
  if (ptr != (char *) NULL) {
    *ptr = '\0';
    path = ptr + 1;
  }
  if (diopts->optval [DI_OPT_DEBUG] > 2) {
    printf ("quota: nfs: host: %s path: %s\n", host, path);
  }
  args.gqa_pathp = path;
  args.gqa_uid = (int) diqinfo->uid;

  rqclnt = clnt_create (host, (unsigned long) RQUOTAPROG,
      (unsigned long) RQUOTAVERS, "udp");
  if (rqclnt == (CLIENT *) NULL) {
    if (diopts->optval [DI_OPT_DEBUG] > 2) {
      printf ("quota: nfs: create failed %d\n", errno);
    }
    return;
  }
  rqclnt->cl_auth = authunix_create_default ();
  if (diopts->optval [DI_OPT_DEBUG] > 5) {
    printf ("quota: xdr_quota_get/rslt\n");
  }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
/* how many ways does clang complain about sort-of-valid code? */
#pragma clang diagnostic ignored "-Wincompatible-function-pointer-types"
#pragma clang diagnostic ignored "-Wcast-function-type-strict"
#pragma clang diagnostic ignored "-Wcast-function-type"
#pragma gcc diagnostic push
#pragma gcc diagnostic ignored "-Wcast-function-type"  /* works? */
/* gcc14 on macos complains also, but the pragma maybe does not work */
/* it's an old interface and xdrproc_t isn't quite defined correctly */
/* i will attempt to clean this up at a later date */
  clnt_stat = clnt_call (rqclnt, (unsigned long) RQUOTAPROC_GETQUOTA,
      (xdrproc_t) xdr_quota_get, (caddr_t) &args,
      (xdrproc_t) xdr_quota_rslt, (caddr_t) &result, timeout);
#pragma clang diagnostic pop
#pragma gcc diagnostic pop
  if (clnt_stat != RPC_SUCCESS) {
    if (diopts->optval [DI_OPT_DEBUG] > 2) {
      printf ("quota: nfs: not success\n");
    }
    if (rqclnt->cl_auth) {
/* MacOS does not declare ah_destroy with modern function signatures */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wdeprecated-non-prototype"
      auth_destroy (rqclnt->cl_auth);
#pragma clang diagnostic pop
    }
    clnt_destroy (rqclnt);
    return;
  }

# if _mem_struct_getquota_rslt_gqr_status
  quotastat = (int) result.gqr_status;
# else
  quotastat = (int) result.status;
# endif
  if (quotastat == 1) {
# if _mem_struct_getquota_rslt_gqr_rquota
    rptr = &result.gqr_rquota;
# else
    rptr = &result.getquota_rslt_u.gqr_rquota;
# endif

    if (diopts->optval [DI_OPT_DEBUG] > 2) {
      printf ("quota: nfs: status 1\n");
      printf ("quota: nfs: rq_bsize: %d\n", rptr->rq_bsize);
      printf ("quota: nfs: rq_active: %d\n", rptr->rq_active);
    }

    dinum_mul_uu (&diqinfo->values [DI_QUOTA_LIMIT],
        (di_ui_t) rptr->rq_bhardlimit, (di_ui_t) rptr->rq_bsize);
    dinum_mul_uu (&tsize, (di_ui_t) rptr->rq_bsoftlimit,
        (di_ui_t) rptr->rq_bsize);
    if (dinum_cmp_s (&tsize, (di_si_t) 0) != 0 &&
        dinum_cmp (&tsize, &diqinfo->values [DI_QUOTA_LIMIT]) < 0) {
      dinum_set (&diqinfo->values [DI_QUOTA_LIMIT], &tsize);
    }
    if (dinum_cmp_s (&diqinfo->values [DI_QUOTA_LIMIT], (di_si_t) 0) != 0) {
      dinum_mul_uu (&diqinfo->values [DI_QUOTA_USED],
          (di_ui_t) rptr->rq_curblocks, (di_ui_t) rptr->rq_bsize);
    }

    dinum_set_u (&diqinfo->values [DI_QUOTA_ILIMIT],
        (di_ui_t) rptr->rq_fhardlimit);
    dinum_set_s (&tsize, (di_si_t) rptr->rq_fsoftlimit);
    if (dinum_cmp_s (&tsize, (di_si_t) 0) != 0 &&
        dinum_cmp (&tsize, &diqinfo->values [DI_QUOTA_ILIMIT]) < 0) {
      dinum_set (&diqinfo->values [DI_QUOTA_ILIMIT], &tsize);
    }
    if (dinum_cmp_s (&diqinfo->values [DI_QUOTA_ILIMIT], (di_si_t) 0) != 0) {
      dinum_set_u (&diqinfo->values [DI_QUOTA_IUSED],
          (di_ui_t) rptr->rq_curfiles);
    }
  }

  if (rqclnt->cl_auth) {
/* MacOS does not declare ah_destroy with modern function signatures */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wdeprecated-non-prototype"
    auth_destroy (rqclnt->cl_auth);
#pragma clang diagnostic pop
  }
  clnt_destroy (rqclnt);

  dinum_clear (&tsize);
}
#endif  /* have std nfs quotas */

#if _has_std_quotas
static void
di_process_quotas (di_data_t *di_data, const char *tag, di_quota_t *diqinfo,
                  int rc, int xfsflag, qdata_t *qdata)
{
  dinum_t        quot_block_sz;
  dinum_t        qspace_block_sz;
  dinum_t        tsize;
  dinum_t        tlimit;
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  dinum_init (&quot_block_sz);
  dinum_set_u (&quot_block_sz, (di_ui_t) DI_QUOT_BLOCK_SIZE);
  dinum_init (&qspace_block_sz);
  dinum_set_u (&qspace_block_sz, (di_ui_t) DI_QUOT_BLOCK_SIZE);
  dinum_init (&tsize);
  dinum_init (&tlimit);

  if (diopts->optval [DI_OPT_DEBUG] > 5) {
    printf ("quota: di_process_quotas\n");
  }
# if _lib_vquotactl
  dinum_set_u (&quot_block_sz, (di_ui_t) 1);
  dinum_set_u (&qspace_block_sz, (di_ui_t) 1);
# endif
# if _mem_struct_dqblk_dqb_curspace
  dinum_set_u (&qspace_block_sz, (di_ui_t) 1);
# endif
  if (xfsflag) {
    dinum_set_u (&quot_block_sz, (di_ui_t) 512);
    dinum_set_u (&qspace_block_sz, (di_ui_t) 512);
  }

  if (rc == 0) {
    dinum_set_u (&tsize, (di_ui_t) 0);
    dinum_set_u (&tlimit, (di_ui_t) 0);
    if (diopts->optval [DI_OPT_DEBUG] > 1) {
      char tbuff [100];
      dinum_str (&quot_block_sz, tbuff, sizeof (tbuff));
      printf ("# diquota: blocksize: %s\n", tbuff);
    }
    if (xfsflag) {
# if _typ_fs_disk_quota_t
      if (diopts->optval [DI_OPT_DEBUG] > 1) { printf ("# diquota: fs_disk_quota_t\n"); }
        dinum_set_u (&tsize, (di_ui_t) qdata->xfsqinfo.d_blk_hardlimit);
# endif
      ;
    } else {
# if _lib_vquotactl
      if (diopts->optval [DI_OPT_DEBUG] > 1) { printf ("# diquota: vquotactl\n"); }
      dinum_set_u (&tsize, (di_ui_t) qdata->vqval.vqvalptr->values [DI_QUOTA_LIMIT]);
# endif
# if _typ_struct_quotaval
      if (diopts->optval [DI_OPT_DEBUG] > 1) { printf ("# diquota: struct quotaval\n"); }
      dinum_set_u (&tsize, (di_ui_t) qdata->qval.qbval.qv_hardlimit);
# endif
# if _typ_struct_dqblk && ! _lib_vquotactl
      if (diopts->optval [DI_OPT_DEBUG] > 1) { printf ("# diquota: struct dqblk\n"); }
      dinum_set_u (&tsize, (di_ui_t) qdata->qinfo.dqb_bhardlimit);
# endif
    }
    if (diopts->optval [DI_OPT_DEBUG] > 2) {
      char tbuff [100];
      dinum_str (&tsize, tbuff, sizeof (tbuff));
      printf ("quota: %s %s b hard: %s\n", tag, diqinfo->mountpt, tbuff);
    }

    if (dinum_cmp_s (&tsize, (di_si_t) 0) > 0) {
      dinum_mul (&tsize, &quot_block_sz);
      if (dinum_cmp_s (&tsize, (di_si_t) 0) > 0 &&
          (dinum_cmp (&tsize, &diqinfo->values [DI_QUOTA_LIMIT]) < 0 ||
          dinum_cmp_s (&diqinfo->values [DI_QUOTA_LIMIT], (di_si_t) 0) == 0)) {
        if (diopts->optval [DI_OPT_DEBUG] > 2) {
          char  tbuffa [100];
          char  tbuffb [100];
          dinum_str (&quot_block_sz, tbuffa, sizeof (tbuffa));
          dinum_str (&tsize, tbuffb, sizeof (tbuffb));
          printf ("quota: using b hard: %s (%s)\n", tbuffb, tbuffa);
        }
        dinum_set (&diqinfo->values [DI_QUOTA_LIMIT], &tsize);
        dinum_set (&tlimit, &tsize);
      }
    }

    if (xfsflag) {
# if _typ_fs_disk_quota_t
      dinum_set_u (&tsize, (di_ui_t) qdata->xfsqinfo.d_blk_softlimit);
# endif
      ;
    } else {
# if _lib_vquotactl  /* no soft limit, use hard */
      dinum_set_u (&tsize, (di_ui_t) qdata->vqval.vqvalptr->values [DI_QUOTA_LIMIT]);
# endif
# if _typ_struct_quotaval
      dinum_set_u (&tsize, (di_ui_t) qdata->qval.qbval.qv_softlimit);
# endif
# if _typ_struct_dqblk && ! _lib_vquotactl
      dinum_set_u (&tsize, (di_ui_t) qdata->qinfo.dqb_bsoftlimit);
# endif
    }
    if (diopts->optval [DI_OPT_DEBUG] > 2) {
      char tbuffa [100];
      dinum_str (&quot_block_sz, tbuffa, sizeof (tbuffa));
      printf ("quota: %s %s b soft: %s\n", tag, diqinfo->mountpt, tbuffa);
    }
    if (dinum_cmp_s (&tsize, (di_si_t) 0) > 0) {
      dinum_mul (&tsize, &quot_block_sz);
      if (dinum_cmp_s (&tsize, (di_si_t) 0) > 0 &&
          (dinum_cmp (&tsize, &diqinfo->values [DI_QUOTA_LIMIT]) < 0 ||
           dinum_cmp_s (&diqinfo->values [DI_QUOTA_LIMIT], (di_si_t) 0) == 0)) {
        if (diopts->optval [DI_OPT_DEBUG] > 2) {
          char  tbuffa [100];
          char  tbuffb [100];
          dinum_str (&quot_block_sz, tbuffa, sizeof (tbuffa));
          dinum_str (&tsize, tbuffb, sizeof (tbuffb));
          printf ("quota: using b soft: %s (%s)\n", tbuffb, tbuffa);
        }
        dinum_set (&diqinfo->values [DI_QUOTA_LIMIT], &tsize);
        dinum_set (&tlimit, &tsize);
      }
    }

    /* any quota set? */
    if (dinum_cmp_s (&tlimit, (di_si_t) 0) == 0) {
      if (diopts->optval [DI_OPT_DEBUG] > 2) {
        printf ("quota: %s %s no quota\n", tag, diqinfo->mountpt);
      }
      return;
    }

    if (diopts->optval [DI_OPT_DEBUG] > 1) {
      char  tbuff [100];
      dinum_str (&qspace_block_sz, tbuff, sizeof (tbuff));
      printf ("# diquota: space block size: %s\n", tbuff);
    }
    dinum_set_u (&tlimit, (di_ui_t) 0);
    if (xfsflag) {
# if _typ_fs_disk_quota_t
      if (diopts->optval [DI_OPT_DEBUG] > 1) { printf ("# diquota: fs_disk_quota_t\n"); }
      dinum_set_u (&tsize, (di_ui_t) qdata->xfsqinfo.d_bcount);
# endif
      ;
    } else {
# if _lib_vquotactl
      if (diopts->optval [DI_OPT_DEBUG] > 1) { printf ("# diquota: vquotactl\n"); }
      dinum_set_u (&tsize, (di_ui_t) qdata->vqval.vqvalptr->usage);
# endif
# if _typ_struct_quotaval
      if (diopts->optval [DI_OPT_DEBUG] > 1) { printf ("# diquota: struct quotaval\n"); }
      dinum_set_u (&tsize, (di_ui_t) qdata->qval.qbval.qv_usage);
# endif
# if _mem_struct_dqblk_dqb_curspace
      if (diopts->optval [DI_OPT_DEBUG] > 1) { printf ("# diquota: dqb_curspace\n"); }
      dinum_set_u (&tsize, (di_ui_t) qdata->qinfo.dqb_curspace);
# endif
# if _mem_struct_dqblk_dqb_curblocks && ! _lib_vquotactl
      if (diopts->optval [DI_OPT_DEBUG] > 1) { printf ("# diquota: dqb_curblocks\n"); }
      dinum_set_u (&tsize, (di_ui_t) qdata->qinfo.dqb_curblocks);
# endif
    }

    dinum_mul (&tsize, &qspace_block_sz);
    if (dinum_cmp (&tsize, &diqinfo->values [DI_QUOTA_USED]) > 0 ||
        dinum_cmp_s (&diqinfo->values [DI_QUOTA_USED], (di_si_t) 0) == 0) {
      dinum_set (&diqinfo->values [DI_QUOTA_USED], &tsize);
    }

    if (diopts->optval [DI_OPT_DEBUG] > 2) {
      char  tbuffa [100];
      char  tbuffb [100];
      dinum_str (&diqinfo->values [DI_QUOTA_USED], tbuffa, sizeof (tbuffa));
      dinum_str (&diqinfo->values [DI_QUOTA_LIMIT], tbuffb, sizeof (tbuffb));
      printf ("quota: %s %s used: %s limit: %s\n", tag, diqinfo->mountpt,
          tbuffa, tbuffb);
    }

# if ! _lib_vquotactl   /* no inode limits */
    if (xfsflag) {
#  if _typ_fs_disk_quota_t
      dinum_set_u (&tsize, (di_ui_t) qdata->xfsqinfo.d_ino_hardlimit);
#  endif
      ;
    } else {
#  if _typ_struct_quotaval
      dinum_set_u (&tsize, (di_ui_t) qdata->qval.qival.qv_hardlimit);
#  endif
#  if _typ_struct_dqblk && ! _lib_vquotactl
      dinum_set_u (&tsize, (di_ui_t) qdata->qinfo.dqb_ihardlimit);
#  endif
    }
    if (dinum_cmp_s (&tsize, (di_si_t) 0) > 0 &&
        (dinum_cmp (&tsize, &diqinfo->values [DI_QUOTA_ILIMIT]) < 0 ||
        dinum_cmp_s (&diqinfo->values [DI_QUOTA_ILIMIT], (di_si_t) 0) == 0)) {
      dinum_set (&diqinfo->values [DI_QUOTA_ILIMIT], &tsize);
      dinum_set (&tlimit, &tsize);
    }
    if (diopts->optval [DI_OPT_DEBUG] > 2) {
      char tbuffa [100];
      dinum_str (&quot_block_sz, tbuffa, sizeof (tbuffa));
      printf ("quota: %s %s i hard: %s\n", tag, diqinfo->mountpt, tbuffa);
    }

    if (xfsflag) {
#  if _typ_fs_disk_quota_t
      dinum_set_u (&tsize, (di_ui_t) qdata->xfsqinfo.d_ino_softlimit);
#  endif
      ;
    } else {
#  if _typ_struct_quotaval
      dinum_set_u (&tsize, (di_ui_t) qdata->qval.qival.qv_softlimit);
#  endif
#  if _typ_struct_dqblk && ! _lib_vquotactl
      dinum_set_u (&tsize, (di_ui_t) qdata->qinfo.dqb_isoftlimit);
#  endif
    }
    if (dinum_cmp_s (&tsize, (di_si_t) 0) > 0 &&
        (dinum_cmp (&tsize, &diqinfo->values [DI_QUOTA_ILIMIT]) < 0 ||
        dinum_cmp_s (&diqinfo->values [DI_QUOTA_ILIMIT], (di_si_t) 0) == 0)) {
      dinum_set (&diqinfo->values [DI_QUOTA_ILIMIT], &tsize);
      dinum_set (&tlimit, &tsize);
    }
    if (diopts->optval [DI_OPT_DEBUG] > 2) {
      char  tbuffa [100];
      dinum_str (&tsize, tbuffa, sizeof (tbuffa));
      printf ("quota: %s %s i soft: %s\n", tag, diqinfo->mountpt, tbuffa);
    }

      /* any quota set? */
    if (dinum_cmp_s (&tlimit, (di_si_t) 0) == 0) {
      if (diopts->optval [DI_OPT_DEBUG] > 2) {
        printf ("quota: %s %s no inode quota\n", tag, diqinfo->mountpt);
      }
      return;
    }

    if (xfsflag) {
#  if _typ_fs_disk_quota_t
      dinum_set_u (&tsize, (di_ui_t) qdata->xfsqinfo.d_icount);
#  endif
      ;
    } else {
#  if _typ_struct_quotaval
      dinum_set_u (&tsize, (di_ui_t) qdata->qval.qival.qv_usage);
#  endif
#  if ! _lib_quota_open && ! _lib_vquotactl
      dinum_set_u (&tsize, (di_ui_t) qdata->qinfo.dqb_curinodes);
#  endif
    }
    if (dinum_cmp (&tsize, &diqinfo->values [DI_QUOTA_IUSED]) > 0 ||
        dinum_cmp_s (&diqinfo->values [DI_QUOTA_IUSED], (di_si_t) 0) == 0) {
      dinum_set (&diqinfo->values [DI_QUOTA_IUSED], &tsize);
    }
    if (diopts->optval [DI_OPT_DEBUG] > 2) {
      char  tbuffa [100];
      dinum_str (&tsize, tbuffa, sizeof (tbuffa));
      printf ("quota: %s %s i used: %s\n", tag, diqinfo->mountpt, tbuffa);
    }
# endif /* ! _lib_vquotactl */
  } else {
    if (diopts->optval [DI_OPT_DEBUG] > 2) {
      printf ("quota: %s %s errno %d\n", tag, diqinfo->mountpt, errno);
    }
  }

  dinum_clear (&quot_block_sz);
  dinum_clear (&qspace_block_sz);
  dinum_clear (&tsize);
  dinum_clear (&tlimit);
}
#endif /* _has_std_quotas */
