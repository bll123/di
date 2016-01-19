/*
 * Copyright 2016 Brad Lanam, Walnut Creek, CA
 */

#include "config.h"
#include "configtcl.h"
#include "di.h"
#include "dimain.h"
#include "display.h"
#include "version.h"
#include "options.h"

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
static char *diproc _((int, char **));

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
  char              **argv;
  char              *rv;
  char              *ptr;
  int               ptrlen;
  int               i;

  if (objc < 2) {
    Tcl_WrongNumArgs (interp, 1, objv, "[options] partition-name");
    return TCL_ERROR;
  }
  argv = (char **) malloc (sizeof(char *) * objc);
  for (i = 0; i < objc; ++i) {
    ptr = Tcl_GetStringFromObj (objv[i], &ptrlen);
    argv[i] = strdup (ptr);
  }
  argv[objc] = NULL;

  rv = diproc (objc, argv);
  for (i = 0; i < objc; ++i) {
    free (argv[i]);
  }
  free (argv);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(rv, (int) strlen(rv)));
  return TCL_OK;
}

static char *
#if _proto_stdc
diproc (int argc, char **argv)
#else
diproc (argc, argv)
    int argc;
    char **argv;
#endif
{
  char      *disp;

  disp = dimainproc (argc, (const char * const *) argv, 1);
  return disp;
}

