/*
 * Copyright 2016 Brad Lanam, Walnut Creek, CA
 */

#include "config.h"
#include "configtcl.h"
#include "di.h"
#include "dimain.h"
#include "version.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _sys_types \
    && ! defined (DI_INC_SYS_TYPES_H) /* xenix */
# define DI_INC_SYS_TYPES_H
# include <sys/types.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
#endif
#if _hdr_unistd
# include <unistd.h>
#endif
#if _hdr_memory
# include <memory.h>
#endif
#if _hdr_malloc
# include <malloc.h>
#endif
#if _hdr_tcl
# define USE_TCL_STUBS
# include <tcl.h>
#endif
#if _use_mcheck
# include <mcheck.h>
#endif

#if defined (__cplusplus) || defined (c_plusplus)
  extern "C" {
#endif

#if _npt_getenv
  extern char *getenv _((const char *));
#endif

#if defined (__cplusplus) || defined (c_plusplus)
  }
#endif

/* end of system specific includes/configurations */

#if defined (__cplusplus) || defined (c_plusplus)
  extern "C" {
#endif

int Diskspace_Init _((Tcl_Interp *));
int diskspaceObjCmd _((ClientData, Tcl_Interp *, int, Tcl_Obj * const []));
static void addListToDict _((Tcl_Interp *, Tcl_Obj *, const char *, char *));
static void addStringToDict _((Tcl_Interp *, Tcl_Obj *, const char *, const char *));
static void addWideToDict _((Tcl_Interp *, Tcl_Obj *, const char *, _fs_size_t));
static char *diproc _((int, const char **, diData_t *));

#if defined (__cplusplus) || defined (c_plusplus)
  }
#endif

int
#if _proto_stdc
Diskspace_Init (Tcl_Interp *interp)
#else
Diskspace_Init (interp)
  Tcl_Interp *interp;
#endif
{
#if _use_mcheck
  mcheck_pedantic (NULL);
  mtrace ();
#endif
  if (! Tcl_InitStubs (interp, "8.5", 0)) {
    return TCL_ERROR;
  }
  Tcl_CreateObjCommand (interp, "diskspace", diskspaceObjCmd,
      (ClientData) NULL, NULL);
  Tcl_PkgProvide (interp, "diskspace", DI_VERSION);
  return TCL_OK;
}

int
#if _proto_stdc
diskspaceObjCmd (
  ClientData cd,
  Tcl_Interp *interp,
  int objc,
  Tcl_Obj * const objv[]
  )
#else
diskspaceObjCmd (interp, objc, objv)
  ClientData        cd;
  Tcl_Interp        *interp;
  int               objc;
  Tcl_Obj * const   objv[];
#endif
{
  const char        **argv;
  char              *rv;
  const char        *tptr;
  char              *ttptr;
  char              **dispargs;
  int               i;
  diData_t          diData;
  Tcl_Obj           *dictObj;
  Tcl_Obj           *tempDictObj;
  Tcl_Obj           *mountKey;
  diDiskInfo_t      *diskInfo;
  diDiskInfo_t      *dinfo;


  /* using malloc here causes tcl to crash */
  /* rather weird, as this is my value, not Tcl's */
  argv = (const char **) ckalloc (sizeof(const char *) * (Size_t) (objc + 1));
  for (i = 0; i < objc; ++i) {
    tptr = Tcl_GetString (objv[i]);
    argv[i] = tptr;
  }
  argv[objc] = NULL;

  rv = diproc (objc, argv, &diData);
  if (diData.options.exitFlag == DI_EXIT_FAIL) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("malloc failure", -1));
    Tcl_SetErrorCode(interp, "diskspace", NULL);
    ckfree (argv);
    return TCL_ERROR;
  }
  if (diData.options.exitFlag == DI_EXIT_WARN) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("invalid arguments", -1));
    Tcl_SetErrorCode(interp, "diskspace", NULL);
    ckfree (argv);
    return TCL_ERROR;
  }
  if (diData.options.exitFlag == DI_EXIT_OK) {
    ckfree (argv);
    return TCL_OK;
  }
  diskInfo = diData.diskInfo;

  dictObj = Tcl_NewDictObj ();
  dispargs = (char **) malloc (sizeof(char *) *
      (Size_t) diData.count);
  if (rv != (char *) NULL) {
    ttptr = strtok (rv, "\n");
  } else {
    ttptr = (char *) NULL;
  }

  for (i = 0; i < diData.count; ++i) {
    dinfo = &(diskInfo [diskInfo [i].sortIndex[DI_TOT_SORT_IDX]]);
    dispargs[i] = (char *) NULL;
    if (! dinfo->doPrint) {
      continue;
    }
    dispargs[i] = ttptr;
    if (rv != (char *) NULL) {
      ttptr = strtok (NULL, "\n");
    }
  }

  for (i = 0; i < diData.count; ++i) {
    dinfo = &(diskInfo [diskInfo [i].sortIndex[DI_TOT_SORT_IDX]]);
    if (! dinfo->doPrint) {
      continue;
    }

    tempDictObj = Tcl_NewDictObj ();
    addStringToDict (interp, tempDictObj, "device", dinfo->special);
    addStringToDict (interp, tempDictObj, "fstype", dinfo->fsType);
    addWideToDict (interp, tempDictObj, "total", dinfo->totalSpace);
    addWideToDict (interp, tempDictObj, "free", dinfo->freeSpace);
    addWideToDict (interp, tempDictObj, "available", dinfo->availSpace);
    addWideToDict (interp, tempDictObj, "totalinodes", dinfo->totalInodes);
    addWideToDict (interp, tempDictObj, "freeinodes", dinfo->freeInodes);
    addWideToDict (interp, tempDictObj, "availableinodes", dinfo->availInodes);
    addStringToDict (interp, tempDictObj, "mountoptions", dinfo->options);
    if (dispargs[i] != (char *) NULL) {
      addListToDict (interp, tempDictObj, "display", dispargs[i]);
    }
    mountKey = Tcl_NewStringObj (dinfo->name, -1);
    Tcl_DictObjPut (interp, dictObj, mountKey, tempDictObj);
  }

  Tcl_SetObjResult(interp, dictObj);
  if (rv != (char *) NULL) {
    free (rv);
  }
  free (dispargs);
  cleanup (&diData);
  ckfree (argv);
  return TCL_OK;
}

static void
#if _proto_stdc
addListToDict (Tcl_Interp *interp, Tcl_Obj *dict,
        const char *nm, char *val)
#else
addListToDict (interp, dict, nm, val)
  Tcl_Interp *interp;
  Tcl_Obj *dict;
  const char *nm;
  char *val;
#endif
{
  Tcl_Obj           *tempObj;
  Tcl_Obj           *tempObj1;
  Tcl_Obj           *tempObj2;
  char              *dptr;

  tempObj2 = Tcl_NewListObj (0, NULL);
  dptr = strtok (val, "	"); /* tab character */
  while (dptr != (char *) NULL) {
    tempObj = Tcl_NewStringObj (dptr, -1);
    Tcl_ListObjAppendElement (interp, tempObj2, tempObj);
    dptr = strtok (NULL, "	"); /* tab character */
  }

  tempObj1 = Tcl_NewStringObj (nm, -1);
  Tcl_DictObjPut (interp, dict, tempObj1, tempObj2);
}

static void
#if _proto_stdc
addStringToDict (Tcl_Interp *interp, Tcl_Obj *dict,
        const char *nm, const char *val)
#else
addStringToDict (interp, dict, nm, val)
  Tcl_Interp *interp;
  Tcl_Obj *dict;
  const char *nm;
  const char *val;
#endif
{
  Tcl_Obj           *tempObj1;
  Tcl_Obj           *tempObj2;

  tempObj1 = Tcl_NewStringObj (nm, -1);
  tempObj2 = Tcl_NewStringObj (val, -1);
  Tcl_DictObjPut (interp, dict, tempObj1, tempObj2);
}

static void
#if _proto_stdc
addWideToDict (Tcl_Interp *interp, Tcl_Obj *dict,
        const char *nm, _fs_size_t val)
#else
addWideToDict (interp, dict, nm, val)
  Tcl_Interp *interp;
  Tcl_Obj *dict;
  const char *nm;
  _fs_size_t val;
#endif
{
  Tcl_Obj           *tempObj1;
  Tcl_Obj           *tempObj2;
  Tcl_WideInt       wideVal;

  wideVal = (Tcl_WideInt) val;
  tempObj1 = Tcl_NewStringObj (nm, -1);
  tempObj2 = Tcl_NewWideIntObj (wideVal);
  Tcl_DictObjPut (interp, dict, tempObj1, tempObj2);
}


static char *
#if _proto_stdc
diproc (int argc, const char **argv, diData_t *diData)
#else
diproc (argc, argv)
    int argc;
    const char **argv;
    diData_t   *diData;
#endif
{
  char      *disp;

  disp = dimainproc (argc, (const char * const *) argv, 1, diData);
  return disp;
}
