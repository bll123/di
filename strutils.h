#ifndef INC_STRUTILS_H
#define INC_STRUTILS_H

#include "config.h"

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

extern void * di_realloc            (void *, Size_t);
extern void di_trimchar                (char *, int);

# if ! _lib_strdup
extern char *strdup                 (const char *);
# endif

# if ! _lib_strstr
extern char *strstr           (const char *, const char *);
# endif

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_STRUTILS_H */
