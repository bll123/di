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
    && ! defined (_DI_INC_SYS_TYPES_H) /* xenix */
# define _DI_INC_SYS_TYPES_H
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
# include <tcl.h>
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
static void addStringToDict _((Tcl_Interp *, Tcl_Obj *, const char *, const char *));
static void addWideToDict _((Tcl_Interp *, Tcl_Obj *, const char *, _fs_size_t));
static char *diproc _((int, const char **, diData_t **));

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
#ifdef USE_TCL_STUBS
  if (! Tcl_InitStubs (interp, "8.5", 0)) {
    return TCL_ERROR;
  }
#else
  if (! Tcl_PkgRequire (interp, "Tcl", "8.5", 0)) {
    return TCL_ERROR;
  }
#endif
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
  const char        *ptr;
  int               i;
  diData_t          *diDataOut;
  char              *dispPtr;
  Tcl_Obj           *dictObj;
  Tcl_Obj           *tempDictObj;
  Tcl_Obj           *mountKey;
  diDiskInfo_t      *diskInfo;

  /* using malloc here causes tcl to crash */
  argv = (const char **) ckalloc (sizeof(const char *) * (Size_t) objc);
  for (i = 0; i < objc; ++i) {
    ptr = Tcl_GetString (objv[i]);
    argv[i] = ptr;
  }
  argv[objc] = NULL;

  rv = diproc (objc, argv, &diDataOut);
  ckfree (argv);

  dictObj = Tcl_NewDictObj ();
  dispPtr = strtok (rv, "\n");

  diskInfo = diDataOut->diskInfo;
  for (i = 0; i < diDataOut->count; ++i) {
    diDiskInfo_t    *dinfo;

    dinfo = &(diskInfo [diskInfo [i].sortIndex[DI_TOT_SORT_IDX]]);
    if (! dinfo->doPrint) {
      continue;
    }

    tempDictObj = Tcl_NewDictObj ();
    if (dispPtr != (char *) NULL) {
      addStringToDict (interp, tempDictObj, "display", dispPtr);
    }
    addStringToDict (interp, tempDictObj, "device", dinfo->special);
    addStringToDict (interp, tempDictObj, "fstype", dinfo->fsType);
    addWideToDict (interp, tempDictObj, "total", dinfo->totalSpace);
    addWideToDict (interp, tempDictObj, "free", dinfo->freeSpace);
    addWideToDict (interp, tempDictObj, "available", dinfo->availSpace);
    addWideToDict (interp, tempDictObj, "totalinodes", dinfo->totalInodes);
    addWideToDict (interp, tempDictObj, "freeinodes", dinfo->freeInodes);
    addWideToDict (interp, tempDictObj, "availableinodes", dinfo->availInodes);
    addStringToDict (interp, tempDictObj, "mountoptions", dinfo->options);
    mountKey = Tcl_NewStringObj (dinfo->name, -1);
    Tcl_DictObjPut (interp, dictObj, mountKey, tempDictObj);
    dispPtr = strtok (NULL, "\n");
  }

  Tcl_SetObjResult(interp, dictObj);
  free (rv);
  cleanup (diDataOut);
  return TCL_OK;
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
diproc (int argc, const char **argv, diData_t **diDataOut)
#else
diproc (argc, argv)
    int argc;
    const char **argv;
    diData_t   **diDataOut;
#endif
{
  char      *disp;

  disp = dimainproc (argc, (const char * const *) argv, 1, diDataOut);
  return disp;
}
