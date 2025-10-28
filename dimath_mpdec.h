/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#ifndef INC_DIMATH_MPDEC_H
#define INC_DIMATH_MPDEC_H

#include "config.h"

#include "dimath_def.h"

// # pragma clang diagnostic push
// # pragma clang diagnostic ignored "-Wbad-function-cast"

#include <mpdecimal.h>

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif

static mpd_context_t  mpdctx;
static int            mpdinitialized = 0;

typedef mpd_t   *dinum_t;

static inline void
dinum_init (dinum_t *r)
{
  uint32_t    status;

  *r = mpd_qnew ();
  mpd_qset_u64 (*r, 0, &mpdctx, &status);
}

static inline void
dinum_clear (dinum_t *r)
{
  mpd_del (*r);
}

static inline void
dinum_str (const dinum_t *r, char *str, Size_t sz)
{
  uint32_t  status;
  char      *tstr;

  tstr = mpd_qformat (*r, ".0f", &mpdctx, &status);
  snprintf (str, sz, "%s", tstr);
  free (tstr);
}

static inline void
dinum_set (dinum_t *r, const dinum_t *val)
{
  uint32_t    status;

  mpd_qcopy (*r, *val, &status);
}

static inline void
dinum_set_u (dinum_t *r, di_ui_t val)
{
  uint32_t    status;

  mpd_qset_u64 (*r, val, &mpdctx, &status);
}

static inline void
dinum_set_s (dinum_t *r, di_si_t val)
{
  uint32_t    status;

  mpd_qset_i64 (*r, val, &mpdctx, &status);
}

static inline void
dinum_add_u (dinum_t *r, di_ui_t val)
{
  uint32_t    status;

  mpd_qadd_u64 (*r, *r, val, &mpdctx, &status);
}

static inline void
dinum_sub_u (dinum_t *r, di_ui_t val)
{
  uint32_t    status;

  mpd_qsub_u64 (*r, *r, val, &mpdctx, &status);
}

static inline void
dinum_add (dinum_t *r, const dinum_t *val)
{
  uint32_t    status;

  mpd_qadd (*r, *r, *val, &mpdctx, &status);
}

static inline void
dinum_sub (dinum_t *r, const dinum_t *val)
{
  uint32_t    status;

  mpd_qsub (*r, *r, *val, &mpdctx, &status);
}

static inline int
dinum_cmp (const dinum_t *r, const dinum_t *val)
{
  uint32_t    status;

  return mpd_qcmp (*r, *val, &status);
}

static inline int
dinum_cmp_s (const dinum_t *r, di_si_t val)
{
  uint32_t    status;
  mpd_t       *tval;
  int         rc;

  tval = mpd_qnew ();
  mpd_qset_i64 (tval, val, &mpdctx, &status);
  rc = mpd_qcmp (*r, tval, &status);
  mpd_del (tval);
  return rc;
}

static inline void
dinum_mul (dinum_t *r, const dinum_t *val)
{
  uint32_t    status;

  mpd_qmul (*r, *r, *val, &mpdctx, &status);
}

static inline void
dinum_mul_u (dinum_t *r, di_ui_t val)
{
  uint32_t    status;

  mpd_qmul_u64 (*r, *r, val, &mpdctx, &status);
}

static inline void
dinum_mul_uu (dinum_t *r, di_ui_t vala, di_ui_t valb)
{
  uint32_t    status;

  mpd_qset_u64 (*r, 1, &mpdctx, &status);
  mpd_qmul_u64 (*r, *r, vala, &mpdctx, &status);
  mpd_qmul_u64 (*r, *r, valb, &mpdctx, &status);
}

static inline double
dinum_scale (dinum_t *r, dinum_t *val)
{
  double    dval;

  uint32_t    status;
  mpd_t       *result;
  mpd_t       *rem;
  mpd_t       *t;
  int         rc;

  t = mpd_qnew ();
  mpd_qset_i64 (t, 0, &mpdctx, &status);
  rc = mpd_qcmp (*r, t, &status);
  if (rc == 0) {
    mpd_del (t);
    return 0.0;
  }

  result = mpd_qnew ();
  rem = mpd_qnew ();
  mpd_qset_u64 (t, DI_SCALE_PREC, &mpdctx, &status);
  mpd_qmul (t, *r, t, &mpdctx, &status);

  mpd_qdivmod (result, rem, t, *val, &mpdctx, &status);
  dval = (double) mpd_qget_u64 (result, &status);
  dval /= (double) DI_SCALE_PREC;
  mpd_del (result);
  mpd_del (rem);
  mpd_del (t);

  return dval;
}

static inline double
dinum_perc (dinum_t *r, dinum_t *val)
{
  double      dval = 0.0;

  uint32_t    status;
  mpd_t       *quot;
  mpd_t       *rem;
  mpd_t       *t;

  quot = mpd_qnew ();
  rem = mpd_qnew ();
  t = mpd_qnew ();

  mpd_qset_u64 (t, DI_PERC_PRECISION, &mpdctx, &status);
  mpd_qmul (t, *r, t, &mpdctx, &status);
  mpd_qdivmod (quot, rem, t, *val, &mpdctx, &status);
  dval = (double) mpd_qget_u64 (quot, &status);
  dval /= (double) DI_PERC_DIV;
  mpd_del (quot);
  mpd_del (rem);
  mpd_del (t);

  return dval;
}

static inline void
dimath_initialize (void)
{
  if (mpdinitialized == 0) {
    mpd_maxcontext (&mpdctx);
    mpdinitialized = 1;
  }
}

static inline void
dimath_cleanup (void)
{
  if (mpdinitialized == 1) {
//    mpd_maxcontext (&mpdctx);
    mpdinitialized = 0;
  }
}

# pragma clang diagnostic pop

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DIMATH_MPDEC_H */
