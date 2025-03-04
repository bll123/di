/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#ifndef INC_DISTRUTILS_H
#define INC_DISTRUTILS_H

#include "config.h"
#if _hdr_stddef
# include <stddef.h>
#endif

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

extern void * di_realloc (void *, Size_t);
extern void di_trimchar (char *, int);
char * di_strtok (char *str, const char *delim, char **tokstr);
# if ! _lib_stpecpy
char * stpecpy (char *dst, char *end, const char *src);
# endif

# if ! _lib_strdup
extern char * strdup (const char *);
# endif

# if ! _lib_strstr
extern char * strstr (const char *, const char *);
# endif

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DISTRUTILS_H */
