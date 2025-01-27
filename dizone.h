/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#ifndef INC_DIZONE_H
#define INC_DIZONE_H

#include "config.h"
#include "disystem.h"
#include "dioptions.h"

#if _hdr_unistd
# include <unistd.h>
#endif
#if _hdr_zone
# include <zone.h>
#endif

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

#if ! _lib_zone_list
# define zoneid_t       int
# define ZONENAME_MAX   65
#endif

typedef struct {
  zoneid_t    zoneid;
  char        name [ZONENAME_MAX + 1];
  char        rootpath [MAXPATHLEN + 1];
  Size_t      rootpathlen;
} di_zone_summ_t;

typedef struct {
  Uid_t           uid;
  zoneid_t        myzoneid;
  di_zone_summ_t  *zones;
  unsigned int    zoneCount;
  int             globalIdx;
} di_zone_info_t;

di_zone_info_t *di_initialize_zones (di_opt_t *diopts);
void di_free_zones (di_zone_info_t *);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DIZONE_H */
