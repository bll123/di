/*
 * $Id$
 * $Source$
 * Copyright 1994-2013 Brad Lanam, Walnut Creek, CA
 */

#ifndef _INC_DI_H
#define _INC_DI_H

#include "config.h"

/*****************************************************/

#include <stdio.h>
#if _hdr_fcntl \
    && ! defined (_DI_INC_FCNTL_H)  /* xenix */
# define _DI_INC_FCNTL_H
# include <fcntl.h>
#endif
#if _sys_file
# include <sys/file.h>
#endif
#if _sys_types \
    && ! defined (_DI_INC_SYS_TYPES_H) /* xenix */
# define _DI_INC_SYS_TYPES_H
# include <sys/types.h>
#endif
#if _hdr_limits
# include <limits.h>        /* PATH_MAX */
#endif
#if _sys_param
# include <sys/param.h>     /* MAXPATHLEN */
#endif
#if _hdr_zone
# include <zone.h>
#endif

#if ! defined (O_NOCTTY)
# define O_NOCTTY 0
#endif

#if ! defined (MAXPATHLEN)
# if defined (_POSIX_PATH_MAX)
#  define MAXPATHLEN        _POSIX_PATH_MAX
# else
#  if defined (PATH_MAX)
#   define MAXPATHLEN       PATH_MAX
#  endif
#  if defined (LPNMAX)
#   define MAXPATHLEN       LPNMAX
#  endif
# endif
#endif

#if ! defined (MAXPATHLEN)
# define MAXPATHLEN         255
#endif

#if _sys_fstyp                          /* HP-UX, Solaris */
# include <sys/fstyp.h>                 /* FSTYPSZ */
# if defined (FSTYPSZ)
#  define DI_TYPE_LEN       FSTYPSZ
# endif
#endif
#if _sys_mount \
    && ! defined (_DI_INC_SYS_MOUNT)       /* NetBSD */
# define _DI_INC_SYS_MOUNT 1
# include <sys/mount.h>                 /* MFSNAMELEN */
# if ! defined (DI_TYPE_LEN) && defined (MFSNAMELEN)
#  define DI_TYPE_LEN       MFSNAMELEN
# endif
#endif
#if _sys_vfstab                         /* ??? */
# include <sys/vfstab.h>
# if ! defined (DI_TYPE_LEN) && defined (FSTYPSZ)
#  define DI_TYPE_LEN       FSTYPSZ
# endif
#endif

#if ! defined (DI_TYPE_LEN)
# define DI_TYPE_LEN        65
#endif

#if ! _lib_memcpy && ! _define_memcpy
# if ! _lib_bcopy && ! _define_bcopy
   #error No_memcpy/bcopy_available.
# else
#  define memcpy(dst, src, cnt)     (bcopy((src), (dst), (cnt)), dst)
# endif
#endif

#if ! _lib_memset && ! _define_memset
# if ! _lib_bzero && ! _define_bzero
   #error No_memset/bzero_available.
# else
#  define memset(s,c,n)    (bzero ((s), (n)), s)
# endif
#endif

#define DI_NAME_LEN            MAXPATHLEN
#define DI_SPEC_NAME_LEN       MAXPATHLEN
#define DI_OPT_LEN             MAXPATHLEN
#define DI_MNT_TIME_LEN        24

#if _siz_long_long >= 4
  typedef unsigned long long _fs_size_t;
  typedef long long _s_fs_size_t;
# define DI_LL "ll"
# define DI_LLu "llu"
#else
  typedef unsigned long _fs_size_t;
  typedef long _s_fs_size_t;
# define DI_LL "l"
# define DI_LLu "lu"
#endif
#if _siz_long_double >= 8
  typedef long double _print_size_t;
# define DI_Lf "Lf"
#else
  typedef double _print_size_t;
# define DI_Lf "f"
#endif

typedef unsigned long __ulong;

#if ! defined (TRUE)
# define TRUE             1
#endif
#if ! defined (FALSE)
# define FALSE            0
#endif

#define DI_PRNT_IGNORE      0
#define DI_PRNT_OK          1
#define DI_PRNT_BAD         2
#define DI_PRNT_OUTOFZONE   3
#define DI_PRNT_EXCLUDE     4
#define DI_PRNT_FORCE       5
#define DI_PRNT_SKIP        6

#define DI_MAIN_SORT_IDX    0
#define DI_TOT_SORT_IDX     1

typedef struct
{
    unsigned int    sortIndex [2];
    _fs_size_t      totalSpace;
    _fs_size_t      freeSpace;
    _fs_size_t      availSpace;
    _fs_size_t      totalInodes;
    _fs_size_t      freeInodes;
    _fs_size_t      availInodes;
    __ulong         st_dev;                      /* disk device number       */
    __ulong         sp_dev;                      /* special device number    */
    __ulong         sp_rdev;                     /* special rdev #           */
    char            doPrint;                     /* do we want to print      */
                                                 /* this entry?              */
    char            printFlag;                   /* print flags              */
    char            isLocal;                     /* is this mount point      */
                                                 /* local?                   */
    char            isReadOnly;                  /* is this mount point      */
                                                 /* read-only?               */
    char            isLoopback;                  /* lofs or none fs type?    */
    char            name [DI_NAME_LEN + 1];         /* mount point           */
    char            special [DI_SPEC_NAME_LEN + 1]; /* special device name   */
    char            fsType [DI_TYPE_LEN + 1];       /* type of file system   */
    char            options [DI_OPT_LEN + 1];
    char            mountTime [DI_MNT_TIME_LEN + 1];
} diDiskInfo_t;

typedef struct
{
    char            *special;
    char            *name;
    char            *type;
    Uid_t           uid;
    Gid_t           gid;
    _fs_size_t      blockSize;
    _fs_size_t      limit;
    _fs_size_t      used;
    _fs_size_t      ilimit;
    _fs_size_t      iused;
} diQuota_t;

typedef struct
{
    int    count;
    char   **list;
} iList_t;

#if ! _lib_zone_list
# define zoneid_t       int
# define ZONENAME_MAX   65
#endif

typedef struct {
    zoneid_t    zoneid;
    char        name [ZONENAME_MAX + 1];
    char        rootpath [MAXPATHLEN + 1];
    Size_t      rootpathlen;
} zoneSummary_t;

typedef struct {
    Uid_t           uid;
    zoneid_t        myzoneid;
    zoneSummary_t   *zones;
    Uint_t          zoneCount;
    char            zoneDisplay [MAXPATHLEN + 1];
    int             globalIdx;
} zoneInfo_t;

#define DI_SORT_MAX             10

typedef struct {
    const char      *formatString;
    _print_size_t   dispBlockSize;
    _print_size_t   baseDispSize;
    unsigned int    baseDispIdx;
    char            sortType [DI_SORT_MAX + 1];
    unsigned int    posix_compat;
    unsigned int    quota_check;
    unsigned int    csv_output;
    unsigned int    excludeLoopback;
    unsigned int    printTotals;
    unsigned int    printDebugHeader;
    unsigned int    printHeader;
    unsigned int    displayAll;
    unsigned int    localOnly;
    unsigned int    dontResolveSymlink;
} diOptions_t;

typedef struct {
    Size_t       inodeWidth;
    Size_t       maxMntTimeString;
    Size_t       maxMountString;
    Size_t       maxOptString;
    Size_t       maxSpecialString;
    Size_t       maxTypeString;
    Size_t       width;
    char         *dispBlockLabel;
    char         blockFormat [15];
    char         blockFormatNR [15];   /* no radix */
    char         inodeFormat [15];
    char         inodeLabelFormat [15];
    char         mountFormat [10];
    char         mTimeFormat [15];
    char         optFormat [15];
    char         specialFormat [15];
    char         typeFormat [10];
} diOutput_t;

typedef struct {
    int             count;
    int             haspooledfs;
    int             disppooledfs;
    int             totsorted;
    diOptions_t     options;
    diOutput_t      output;
    diDiskInfo_t    *diskInfo;
    iList_t         ignoreList;
    iList_t         includeList;
    zoneInfo_t      zoneInfo;
} diData_t;

# if defined (__cplusplus) || defined (c_plusplus)
   extern "C" {
# endif

#if ! _dcl_errno
  extern int errno;
#endif

 /* digetentries.c */
extern int  di_getDiskEntries       _((diDiskInfo_t **, int *));
 /* digetinfo.c */
extern void di_getDiskInfo          _((diDiskInfo_t **, int *));
 /* diquota.c */
extern void diquota                 _((diQuota_t *));
 /* strdup.c */
# if ! _lib_strdup
extern char *strdup                 _((const char *));
# endif
 /* strstr.c */
# if ! _lib_strstr
extern char *strstr                 _((const char *, const char *));
# endif
 /* trimchar.c */
extern void trimChar                _((char *, int));
 /* realloc.c */
extern _pvoid di__realloc           _((_pvoid, Size_t));
 /* didiskutil.c */
extern void di_initDiskInfo _((diDiskInfo_t *));
extern void di_saveBlockSizes _((diDiskInfo_t *, _fs_size_t, _fs_size_t, _fs_size_t, _fs_size_t));
extern void di_saveInodeSizes _((diDiskInfo_t *, _fs_size_t, _fs_size_t, _fs_size_t));
#if _lib_getmntent \
    && ! _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat \
    && ! _lib_mntctl \
    && ! _class_os__Volumes
extern char *chkMountOptions        _((const char *, const char *));
#endif
extern void convertMountOptions     _((unsigned long, diDiskInfo_t *));
extern void convertNFSMountOptions  _((long, long, long, diDiskInfo_t *));
extern void di_testRemoteDisk       _((diDiskInfo_t *));
extern int  di_isPooledFs           _((diDiskInfo_t *));
extern int  di_isLoopbackFs         _((diDiskInfo_t *));
extern Size_t di_mungePoolName      _((char *));

# if defined (__cplusplus) || defined (c_plusplus)
   }
# endif

     /* macro for gettext() */
#ifndef DI_GT
# if _enable_nls
#  define DI_GT(args) gettext(args)
# else
#  define DI_GT(args) (args)
# endif
#endif

# if _lib_sys_dollar_device_scan && _lib_sys_dollar_getdviw

/* Thanks to Craig Berry's VMS::Device for the VMS code. */

typedef struct {
  short   buflen;          /* Length of output buffer */
  short   itmcode;         /* Item code */
  void    *buffer;         /* Buffer address */
  void    *retlen;         /* Return length address */
} VMS_ITMLST;

# define DVI_IS_STRING 1
# define DVI_IS_LONGWORD 2
# define DVI_IS_QUADWORD 3
# define DVI_IS_WORD 4
# define DVI_IS_BYTE 5
# define DVI_IS_VMSDATE 6
# define DVI_IS_BITMAP 7  /* Each bit in the return value indicates something */
# define DVI_IS_ENUM 8    /* Each returned value has a name, and we ought to */
                          /* return the name instead of the value */
# define DVI_IS_ODD 9     /* A catchall */


# define DVI_IS_INPUT (1<<0)
# define DVI_IS_OUTPUT (1<<1)
# define DVI_ENT(a, b, c) {#a, DVI$_##a, b, c, DVI_IS_OUTPUT}

typedef struct {
  char *name;         /* Pointer to the item name */
  int  syscallValue;  /* Value to use in the getDVI item list */
  int  bufferLen;     /* Length the return va buf needs to be. (no nul */
                      /* terminators, so must be careful with the return */
                      /* values. */
  int  returnType;    /* Type of data the item returns */
  int  inOut;         /* Is this an input or an output item? */
} genericID_t ;

/* Macro to fill in a 'traditional' item-list entry */
# define init_itemlist(ile, length, code, bufaddr, retlen_addr) \
{ \
    (ile)->buflen = (length); \
    (ile)->itmcode = (code); \
    (ile)->buffer = (bufaddr); \
    (ile)->retlen = (retlen_addr) ;}

# endif

#endif /* _INC_DI_H */
