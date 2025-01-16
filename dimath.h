/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#ifndef DI_INC_DIMATH_H
#define DI_INC_DIMATH_H

#include "config.h"

#if _hdr_stdint
# include <stdint.h>
#endif
#if _hdr_inttypes
# include <inttypes.h>
#endif
#if _hdr_math
# include <math.h>
#endif

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
  /* unknown, this can't happen */
  typedef unsigned long di_unum_t;
  typedef long di_snum_t;
# define DI_INTERNAL_INT
#endif

#if _use_math == DI_GMP
# include <gmp.h>
  typedef mpz_t dinum_t;
  typedef mpq_t didbl_t;
#elif _use_math == DI_TOMMATH
# define MP_WUR
# if _hdr_tommath
#  include <tommath.h>
# endif
# if _hdr_libtommath_tommath
#  include <libtommath/tommath.h>
# endif
  typedef mp_int dinum_t;
  typedef mp_int didbl_t;
# else /* DI_INTERNAL */
  typedef di_unum_t dinum_t;
  typedef double didbl_t;
#endif

#define DI_PERC_PRECISION 1000000
#define DI_PERC_DIV ((double) (DI_PERC_PRECISION / 100));

static inline void
dinum_init (dinum_t *r)
{
#if _use_math == DI_GMP
  mpz_init_set_ui (*r, 0);
#elif _use_math == DI_TOMMATH
  mp_init_u64 (r, 0);
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
#endif
}

static inline void
dinum_set (dinum_t *r, const dinum_t *val)
{
#if _use_math == DI_GMP
  mpz_set (*r, *val);
#elif _use_math == DI_TOMMATH
  mp_copy (val, r);
#else
  *r = *val;
#endif
}

static inline void
dinum_set_u (dinum_t *r, di_unum_t val)
{
#if _use_math == DI_GMP
  mpz_set_ui (*r, val);
#elif _use_math == DI_TOMMATH
  mp_set_u64 (r, val);
#else
  *r = val;
#endif
}

static inline void
dinum_set_s (dinum_t *r, di_snum_t val)
{
#if _use_math == DI_GMP
  mpz_set_si (*r, val);
#elif _use_math == DI_TOMMATH
  mp_set_i64 (r, val);
#else
  *r = val;
#endif
}

static inline void
dinum_add_u (dinum_t *r, di_unum_t val)
{
#if _use_math == DI_GMP
  mpz_t     v;
  mpz_t     t;

  mpz_init_set_ui (v, val);
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
#else
  *r += val;
#endif
}

static inline void
dinum_sub_u (dinum_t *r, di_unum_t val)
{
#if _use_math == DI_GMP
  mpz_t     v;
  mpz_t     t;

  mpz_init_set_ui (v, val);
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
#else
  *r -= val;
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
dinum_cmp_s (const dinum_t *r, di_snum_t val)
{
#if _use_math == DI_GMP
  return mpz_cmp_si (*r, val);
#elif _use_math == DI_TOMMATH
  mp_int      t;

  mp_init (&t);
  mp_set_i64 (&t, val);
  return mp_cmp (r, &t);
#else
  di_snum_t   t;
  int       rc = 0;

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
  mpz_t     t;

  mpz_init (t);
  mpz_set (t, *r);
  mpz_mul (*r, t, *val);
  mpz_clear (t);
#elif _use_math == DI_TOMMATH
  mp_mul (r, (mp_int *) val, r);
#else
  *r *= *val;
#endif
}

static inline void
dinum_mul_u (dinum_t *r, di_unum_t val)
{
#if _use_math == DI_GMP
  mpz_mul_ui (*r, *r, val);
#elif _use_math == DI_TOMMATH
  mp_int    v;

  mp_init (&v);
  mp_set_u64 (&v, val);
  mp_mul (r, &v, r);
  mp_clear (&v);
#else
  *r *= val;
#endif
}

static inline void
dinum_mul_uu (dinum_t *r, di_unum_t vala, di_unum_t valb)
{
#if _use_math == DI_GMP
  mpz_set_ui (*r, 1);
  mpz_mul_ui (*r, *r, vala);
  mpz_mul_ui (*r, *r, valb);
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
#else
  *r = vala * valb;
#endif
}

static inline void
dinum_scale (dinum_t *result, dinum_t *r, dinum_t *val)
{
#if _use_math == DI_GMP
  mpz_cdiv_q (*result, *r, *val);
#elif _use_math == DI_TOMMATH
  mp_int    rem;
  mp_int    t;

  mp_init (&rem);
  mp_div (r, val, result, &rem);

  mp_init_u64 (&t, 0);
  if (mp_cmp (&rem, &t) > 0) {
    mp_int    v;

    mp_init_u64 (&v, 1);
    mp_add (result, &v, result);
    mp_clear (&v);
  }
  mp_clear (&t);
#else
# if defined (DI_INTERNAL_INT)
  di_unum_t    rem;

  *result = *r / *val;
  rem = *r % *val;
  if (rem > 0) {
    *result += 1;
  }
# endif
# if defined (DI_INTERNAL_DOUBLE)
  *result = *r / *val;
#  if _hdr_math
  *result = ceil (*result);
#  endif
# endif
#endif
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
  mpz_mul_ui (t, *r, DI_PERC_PRECISION);
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
#else
  /* in the case of a uint, simply convert and divide */
  dval = (double) *r / (double) *val;
  dval *= 100.0;
#endif

  return dval;
}

static inline void
dinum_str (const dinum_t *r, char *str, size_t sz)
{
#if _use_math == DI_GMP
  gmp_snprintf (str, sz, "%Zd", *r);
#elif _use_math == DI_TOMMATH
  mp_to_decimal (r, str, sz);
#else
# if defined (DI_INTERNAL_DOUBLE)
#  if _siz_long_double > 8
  snprintf (str, sz, "%.0Lf", *r);
#  else
  snprintf (str, sz, "%.0f", *r);
#  endif
# else
#  if _hdr_inttypes && _siz_uint64_t == 8
  snprintf (str, sz, "%" PRIu64, *r);
#  elif _siz_long == 8
  snprintf (str, sz, "%ld", *r);
#  elif _siz_long_long == 8
  snprintf (str, sz, "%lld", *r);
#  else
  snprintf (str, sz, "%d", *r);
#  endif
# endif
#endif
}

#endif /* DI_INC_DIMATH_H */
