#ifndef INC_DIQUOTA_H
#define INC_DIQUOTA_H

#include "config.h"
#include "disystem.h"
#include "dimath.h"

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

/* quota value identifiers */
#define DI_QUOTA_BLOCK_SZ   0
#define DI_QUOTA_LIMIT      1
#define DI_QUOTA_USED       2
#define DI_QUOTA_ILIMIT     3
#define DI_QUOTA_IUSED      4
#define DI_QVAL_MAX         5

typedef struct
{
  char         *filesystem;
  char         *mountpt;
  char         *fstype;
  Uid_t        uid;
  Gid_t        gid;
  dinum_t      values [DI_QVAL_MAX];
} di_quota_t;

extern void diquota (di_quota_t *);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DIQUOTA_H */
