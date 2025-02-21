/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#ifndef INC_DIMATH_H
#define INC_DIMATH_H

#include "config.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stddef
# include <stddef.h>
#endif
#if _hdr_stdint
# include <stdint.h>
#endif
#if _hdr_inttypes
# include <inttypes.h>
#endif

# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wbad-function-cast"

#if _use_math == DI_GMP
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunknown-warning-option"
# pragma clang diagnostic ignored "-Wsign-conversion"
# pragma clang diagnostic ignored "-Wreserved-identifier"
# pragma gcc diagnostic push
# pragma gcc diagnostic ignored "-Wsign-conversion"
# include <gmp.h>
# pragma clang diagnostic pop
# pragma gcc diagnostic pop
  typedef mpz_t dinum_t;
#elif _use_math == DI_TOMMATH
# define MP_WUR
# if _hdr_tommath
#  include <tommath.h>
# endif
# if _hdr_libtommath_tommath
#  include <libtommath/tommath.h>
# endif
  typedef mp_int dinum_t;
#elif _use_math == DI_MPDECIMAL
# include <mpdecimal.h>
  static mpd_context_t  mpdctx;
  static int            mpdinitialized = 0;
  typedef mpd_t   *dinum_t;
# else /* DI_INTERNAL */
  typedef di_unum_t dinum_t;
#endif

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

/* a double has a longer mantissa than an unsigned int, */
/* but the accuracy may be less. */
#if _siz_long_double > 8
  typedef long double di_unum_t;
  typedef long double di_snum_t;
# define DI_INTERNAL_DOUBLE
#elif _siz_double == 8
  typedef double di_unum_t;
  typedef double di_snum_t;
# define DI_INTERNAL_DOUBLE
#elif _siz_uint64_t == 8
  typedef uint64_t di_unum_t;
  typedef int64_t di_snum_t;
# define DI_INTERNAL_INT
#elif _siz_long == 8
  typedef unsigned long di_unum_t;
  typedef long di_snum_t;
# define DI_INTERNAL_INT
#elif _siz_long_long == 8
  typedef unsigned long long di_unum_t;
  typedef long long di_snum_t;
# define DI_INTERNAL_INT
#elif _siz_long == 4
  typedef unsigned long di_unum_t;
  typedef long di_snum_t;
# define DI_INTERNAL_INT
#else
# error "unable to locate a valid type"
#endif

#if _siz_uint64_t == 8
  typedef uint64_t di_ui_t;
  typedef int64_t di_si_t;
#elif _siz_long == 8
  typedef unsigned long di_ui_t;
  typedef long di_si_t;
#elif _siz_long_long == 8
  typedef unsigned long long di_ui_t;
  typedef long long di_si_t;
#elif _siz_long == 4
  typedef unsigned long di_ui_t;
  typedef long di_si_t;
#else
# error "unable to locate a valid type"
#endif

#define DI_PERC_PRECISION 1000000
#define DI_PERC_DIV ( (double) (DI_PERC_PRECISION / 100));
#define DI_SCALE_PREC 1000

static inline void
dinum_init (dinum_t *r)
{
#if _use_math == DI_GMP
  mpz_init_set_ui (*r, (unsigned long) 0);
#elif _use_math == DI_TOMMATH
  mp_init_u64 (r, 0);
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;

  if (mpdinitialized == 0) {
    mpd_maxcontext (&mpdctx);
    mpdinitialized = 1;
  }
  *r = mpd_qnew ();
  mpd_qset_u64 (*r, 0, &mpdctx, &status);
#else
  *r = 0;
#endif
}

static inline void
dinum_clear (dinum_t *r)
{
#if _use_math == DI_GMP
  mpz_clear (*r);
#elif _use_math == DI_TOMMATH
  mp_clear (r);
#elif _use_math == DI_MPDECIMAL
  mpd_del (*r);
#endif
}

static inline void
dinum_str (const dinum_t *r, char *str, Size_t sz)
{
#if _use_math == DI_GMP
  gmp_snprintf (str, sz, "%Zd", *r);
#elif _use_math == DI_TOMMATH
  mp_to_decimal (r, str, sz);
#elif _use_math == DI_MPDECIMAL
  uint32_t  status;
  char      *tstr;

  tstr = mpd_qformat (*r, ".0f", &mpdctx, &status);
  snprintf (str, sz, "%s", tstr);
  free (tstr);
#else
# if defined (DI_INTERNAL_DOUBLE)
#  if _siz_long_double > 8
  Snprintf1 (str, sz, "%.0Lf", *r);
#  else
  Snprintf1 (str, sz, "%.0f", *r);
#  endif
# else
#  if _hdr_inttypes && _siz_uint64_t == 8
  Snprintf1 (str, sz, "%" PRIu64, *r);
#  elif _siz_long == 8
  Snprintf1 (str, sz, "%ld", *r);
#  elif _siz_long_long == 8
  Snprintf1 (str, sz, "%lld", *r);
#  else
  Snprintf1 (str, sz, "%d", *r);
#  endif
# endif
#endif
}

static inline void
dinum_set (dinum_t *r, const dinum_t *val)
{
#if _use_math == DI_GMP
  mpz_set (*r, *val);
#elif _use_math == DI_TOMMATH
  mp_copy (val, r);
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;

  mpd_qcopy (*r, *val, &status);
#else
  *r = *val;
#endif
}

static inline void
dinum_set_u (dinum_t *r, di_ui_t val)
{
#if _use_math == DI_GMP
  mpz_set_ui (*r, (unsigned long) val);
#elif _use_math == DI_TOMMATH
  mp_set_u64 (r, val);
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;

  mpd_qset_u64 (*r, val, &mpdctx, &status);
#else
  *r = (dinum_t) val;
#endif
}

static inline void
dinum_set_s (dinum_t *r, di_si_t val)
{
#if _use_math == DI_GMP
  mpz_set_si (*r, (long) val);
#elif _use_math == DI_TOMMATH
  mp_set_i64 (r, val);
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;

  mpd_qset_i64 (*r, val, &mpdctx, &status);
#else
  *r = (dinum_t) val;
#endif
}

static inline void
dinum_add_u (dinum_t *r, di_ui_t val)
{
#if _use_math == DI_GMP
  mpz_t     v;
  mpz_t     t;

  mpz_init_set_ui (v, (unsigned long) val);
  mpz_init (t);
  mpz_set (t, *r);
  mpz_add (*r, t, v);
  mpz_clear (t);
  mpz_clear (v);
#elif _use_math == DI_TOMMATH
  mp_int    v;

  mp_init_u64 (&v, val);
  mp_add (r, &v, r);
  mp_clear (&v);
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;

  mpd_qadd_u64 (*r, *r, val, &mpdctx, &status);
#else
  *r += (dinum_t) val;
#endif
}

static inline void
dinum_sub_u (dinum_t *r, di_ui_t val)
{
#if _use_math == DI_GMP
  mpz_t     v;
  mpz_t     t;

  mpz_init_set_ui (v, (unsigned long) val);
  mpz_init (t);
  mpz_set (t, *r);
  mpz_sub (*r, t, v);
  mpz_clear (t);
  mpz_clear (v);
#elif _use_math == DI_TOMMATH
  mp_int    v;

  mp_init_u64 (&v, val);
  mp_sub (r, &v, r);
  mp_clear (&v);
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;

  mpd_qsub_u64 (*r, *r, val, &mpdctx, &status);
#else
  *r -= (dinum_t) val;
#endif
}

static inline void
dinum_add (dinum_t *r, const dinum_t *val)
{
#if _use_math == DI_GMP
  mpz_t     t;

  mpz_init (t);
  mpz_set (t, *r);
  mpz_add (*r, t, *val);
  mpz_clear (t);
#elif _use_math == DI_TOMMATH
  mp_add (r, val, r);
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;

  mpd_qadd (*r, *r, *val, &mpdctx, &status);
#else
  *r += *val;
#endif
}

static inline void
dinum_sub (dinum_t *r, const dinum_t *val)
{
#if _use_math == DI_GMP
  mpz_t     t;

  mpz_init (t);
  mpz_set (t, *r);
  mpz_sub (*r, t, *val);
  mpz_clear (t);
#elif _use_math == DI_TOMMATH
  mp_sub (r, val, r);
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;

  mpd_qsub (*r, *r, *val, &mpdctx, &status);
#else
  *r -= *val;
#endif
}

static inline int
dinum_cmp (const dinum_t *r, const dinum_t *val)
{
#if _use_math == DI_GMP
  return mpz_cmp (*r, *val);
#elif _use_math == DI_TOMMATH
  return mp_cmp (r, val);
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;

  return mpd_qcmp (*r, *val, &status);
#else
  int     rc = 0;

  if (*r < *val) {
    rc = -1;
  } else if (*r > *val) {
    rc = 1;
  }
  return rc;
#endif
}

static inline int
dinum_cmp_s (const dinum_t *r, di_si_t val)
{
#if _use_math == DI_GMP
  return mpz_cmp_si (*r, (long) val);
#elif _use_math == DI_TOMMATH
  mp_int      t;
  int         rv;

  mp_init (&t);
  mp_set_i64 (&t, val);
  rv = mp_cmp (r, &t);
  mp_clear (&t);
  return rv;
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;
  mpd_t       *tval;
  int         rc;

  tval = mpd_qnew ();
  mpd_qset_i64 (tval, val, &mpdctx, &status);
  rc = mpd_qcmp (*r, tval, &status);
  mpd_del (tval);
  return rc;
#else
  di_snum_t   t;
  int         rc = 0;

  t = (di_snum_t) *r;
  if (t < val) {
    rc = -1;
  } else if (t > val) {
    rc = 1;
  }
  return rc;
#endif
}

static inline void
dinum_mul (dinum_t *r, const dinum_t *val)
{
#if _use_math == DI_GMP
  mpz_mul (*r, *r, *val);
#elif _use_math == DI_TOMMATH
  mp_mul (r, (mp_int *) val, r);
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;

  mpd_qmul (*r, *r, *val, &mpdctx, &status);
#else
  *r *= *val;
#endif
}

static inline void
dinum_mul_u (dinum_t *r, di_ui_t val)
{
#if _use_math == DI_GMP
  mpz_mul_ui (*r, *r, (unsigned long) val);
#elif _use_math == DI_TOMMATH
  mp_int    v;

  mp_init (&v);
  mp_set_u64 (&v, val);
  mp_mul (r, &v, r);
  mp_clear (&v);
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;

  mpd_qmul_u64 (*r, *r, val, &mpdctx, &status);
#else
  *r *= (dinum_t) val;
#endif
}

static inline void
dinum_mul_uu (dinum_t *r, di_ui_t vala, di_ui_t valb)
{
#if _use_math == DI_GMP
  mpz_set_ui (*r, (unsigned long) 1);
  mpz_mul_ui (*r, *r, (unsigned long) vala);
  mpz_mul_ui (*r, *r, (unsigned long) valb);
#elif _use_math == DI_TOMMATH
  mp_int    t;
  mp_int    v;

  mp_set_u64 (r, 1);
  mp_init (&t);
  mp_init (&v);
  mp_set_u64 (&t, vala);
  mp_set_u64 (&v, valb);
  mp_mul (&t, &v, r);
  mp_clear (&t);
  mp_clear (&v);
#elif _use_math == DI_MPDECIMAL
  uint32_t    status;

  mpd_qset_u64 (*r, 1, &mpdctx, &status);
  mpd_qmul_u64 (*r, *r, vala, &mpdctx, &status);
  mpd_qmul_u64 (*r, *r, valb, &mpdctx, &status);
#else
  *r = (dinum_t) vala;
  *r *= (dinum_t) valb;
#endif
}

static inline double
dinum_scale (dinum_t *r, dinum_t *val)
{
  double    dval;

#if _use_math == DI_GMP
  mpz_t     t;
  mpz_t     result;

  if (mpz_cmp_si (*val, (long) 0) == 0) {
    return 0.0;
  }

  mpz_init_set_ui (t, (unsigned long) DI_SCALE_PREC);
  mpz_init (result);
  mpz_mul (t, t, *r);
  mpz_cdiv_q (result, t, *val);
  dval = mpz_get_d (result);
  dval /= (double) DI_SCALE_PREC;
  mpz_clear (result);
  mpz_clear (t);
#elif _use_math == DI_TOMMATH
  mp_int    rem;
  mp_int    t;
  mp_int    result;

  mp_init_u64 (&t, 0);

  if (mp_cmp (val, &t) == 0) {
    mp_clear (&t);
    return 0.0;
  }

  mp_init (&rem);
  mp_init (&result);
  mp_set_u64 (&t, DI_SCALE_PREC);
  mp_mul (&t, r, &t);

  mp_div (&t, val, &result, &rem);
  dval = mp_get_double (&result);
  dval /= (double) DI_SCALE_PREC;
  mp_clear (&t);
  mp_clear (&rem);
  mp_clear (&result);
#elif _use_math == DI_MPDECIMAL
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
#else
# if defined (DI_INTERNAL_INT)
  dinum_t   t;

  if (*val == 0) {
    return 0.0;
  }
  t = (*r * DI_SCALE_PREC) / *val;
  dval = (double) t;
  dval /= (double) DI_SCALE_PREC;
# endif
# if defined (DI_INTERNAL_DOUBLE)
  if (*val == (dinum_t) 0.0) {
    return (dinum_t) 0.0;
  }
  dval = (double) (*r / *val);
# endif
#endif

  return dval;
}

static inline double
dinum_perc (dinum_t *r, dinum_t *val)
{
  double      dval = 0.0;

#if _use_math == DI_GMP
  mpz_t     quot;
  mpz_t     t;

  mpz_init (quot);
  mpz_init (t);

  /* multiply by a larger value */
  /* so that the double can get rounded appropriately */
  mpz_mul_ui (t, *r, (unsigned long) DI_PERC_PRECISION);
  mpz_tdiv_q (quot, t, *val);
  dval = mpz_get_d (quot);
  dval /= DI_PERC_DIV;

  mpz_clear (quot);
  mpz_clear (t);
#elif _use_math == DI_TOMMATH
  mp_int    quot;
  mp_int    rem;
  mp_int    t;

  mp_init (&quot);
  mp_init (&rem);
  mp_init (&t);

  /* multiply by a larger value */
  /* so that the double can get rounded appropriately */
  mp_set_u64 (&t, DI_PERC_PRECISION);
  mp_mul (r, &t, &t);
  mp_div (&t, val, &quot, &rem);
  dval = mp_get_double (&quot);
  dval /= DI_PERC_DIV;

  mp_clear (&t);
  mp_clear (&quot);
  mp_clear (&rem);
#elif _use_math == DI_MPDECIMAL
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
#else
# if defined (DI_INTERNAL_DOUBLE)
  dval = (double) (*r / *val);
# endif
# if defined (DI_INTERNAL_INT)
  /* in the case of a uint, simply convert and divide */
  dval = (double) *r / (double) *val;
# endif
  dval *= 100.0;
#endif

  return dval;
}

# pragma clang diagnostic pop

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DIMATH_H */
