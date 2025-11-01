/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#ifndef INC_DIMATH_MPDEC_H
#define INC_DIMATH_MPDEC_H

#include "config.h"

#if _use_math == DI_MPDECIMAL

#if _hdr_stdio
# include <stdio.h>
#endif

#include "dimath.h"

#define DIMATH_MPD_DEBUG 0
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wbad-function-cast"

#include <mpdecimal.h>

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif

typedef mpd_t   *dinum_t;

#if DIMATH_MPD_DEBUG
static inline void
dimath_mpd_errprint (const char *txt, uint32_t status)
{
  char  buff [MPD_MAX_FLAG_STRING];

  mpd_snprint_flags (buff, sizeof (buff), status);
  fprintf (stderr, "%s: status: %s\n", txt, buff);
}

static inline void
dimath_mpd_init_chk (const char *txt)
{
  if (! dimathinitialized) {
    fprintf (stderr, "ERR: %s mpd not initialized\n", txt);
    exit (1);
  }
}

#endif

static inline void
dinum_init (dinum_t *r)
{
  uint32_t    status = 0;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("init");
#endif
  *r = mpd_qnew ();
  mpd_qset_u64 (*r, 0, &mpdctx, &status);
}

static inline void
dinum_clear (dinum_t *r)
{
#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("clear");
#endif
  mpd_del (*r);
}

static inline void
dinum_str (const dinum_t *r, char *str, Size_t sz)
{
  uint32_t  status = 0;
  char      *tstr;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("str");
#endif
  tstr = mpd_qformat (*r, ".0f", &mpdctx, &status);
  snprintf (str, sz, "%s", tstr);
  free (tstr);
}

static inline void
dinum_set (dinum_t *r, const dinum_t *val)
{
  uint32_t    status = 0;

  mpd_qcopy (*r, *val, &status);
}

static inline void
dinum_set_u (dinum_t *r, di_ui_t val)
{
  uint32_t    status = 0;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("set_u");
#endif
  mpd_qset_u64 (*r, val, &mpdctx, &status);
}

static inline void
dinum_set_s (dinum_t *r, di_si_t val)
{
  uint32_t    status = 0;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("set_s");
#endif
  mpd_qset_i64 (*r, val, &mpdctx, &status);
}

static inline void
dinum_add_u (dinum_t *r, di_ui_t val)
{
  uint32_t    status = 0;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("add_u");
#endif
  mpd_qadd_u64 (*r, *r, val, &mpdctx, &status);
}

static inline void
dinum_add (dinum_t *r, const dinum_t *val)
{
  uint32_t    status = 0;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("add");
#endif
  mpd_qadd (*r, *r, *val, &mpdctx, &status);
}

static inline void
dinum_sub_u (dinum_t *r, di_ui_t val)
{
  uint32_t    status = 0;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("sub_u");
#endif
  mpd_qsub_u64 (*r, *r, val, &mpdctx, &status);
}

static inline void
dinum_sub (dinum_t *r, const dinum_t *val)
{
  uint32_t    status = 0;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("sub");
#endif
  mpd_qsub (*r, *r, *val, &mpdctx, &status);
}

static inline int
dinum_cmp (const dinum_t *r, const dinum_t *val)
{
  uint32_t    status = 0;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("cmp");
#endif
  return mpd_qcmp (*r, *val, &status);
}

static inline int
dinum_cmp_s (const dinum_t *r, di_si_t val)
{
  uint32_t    status = 0;
  mpd_t       *tval;
  int         rc;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("cmp_s");
#endif
  tval = mpd_qnew ();
  mpd_qset_i64 (tval, val, &mpdctx, &status);
  rc = mpd_qcmp (*r, tval, &status);
  mpd_del (tval);
  return rc;
}

static inline void
dinum_mul (dinum_t *r, const dinum_t *val)
{
  uint32_t    status = 0;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("mul");
#endif
  mpd_qmul (*r, *r, *val, &mpdctx, &status);
}

static inline void
dinum_mul_u (dinum_t *r, di_ui_t val)
{
  uint32_t    status = 0;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("mul_u");
#endif
  mpd_qmul_u64 (*r, *r, val, &mpdctx, &status);
}

static inline void
dinum_mul_uu (dinum_t *r, di_ui_t vala, di_ui_t valb)
{
  uint32_t    status = 0;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("mul_uu");
#endif
  mpd_qset_u64 (*r, (di_ui_t) 1, &mpdctx, &status);
  mpd_qmul_u64 (*r, *r, vala, &mpdctx, &status);
  mpd_qmul_u64 (*r, *r, valb, &mpdctx, &status);
}

static inline double
dinum_scale (dinum_t *r, dinum_t *val)
{
  double      dval;
  uint32_t    status = 0;
  mpd_t       *result;
  mpd_t       *rem;
  mpd_t       *t;
  int         rc;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("scale");
#endif
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
  uint32_t    status = 0;
  mpd_t       *quot;
  mpd_t       *rem;
  mpd_t       *t;

#if DIMATH_MPD_DEBUG
  dimath_mpd_init_chk ("perc");
#endif
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

# pragma clang diagnostic pop

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* _use_math == DI_MPDECIMAL */

#endif /* INC_DIMATH_MPDEC_H */
