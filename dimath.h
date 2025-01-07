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

#if ! _typ_uint64_t
# if _siz_long == 8
  typedef long uint64_t;
# elif _siz_long_long == 8
  typedef long long uint64_t;
# endif
#endif

#if _use_math == DI_GMP
# include <gmp.h>
  typedef mpz_t dinum_t;
  typedef mpq_t didbl_t;
#elif _use_math == DI_TOMMATH
# define MP_WUR
# include <tommath.h>
  typedef mp_int dinum_t;
  typedef mp_int didbl_t;
# else /* DI_UINT64 */
  typedef uint64_t dinum_t;
  typedef double didbl_t;
#endif

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
dinum_set_u (dinum_t *r, uint64_t val)
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
dinum_set_s (dinum_t *r, int64_t val)
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
dinum_add_u (dinum_t *r, uint64_t val)
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
//  mp_int    t;

  mp_init_u64 (&v, val);
//  mp_init (&t);
//  mp_copy (r, &t);
  mp_add (r, &v, r);
//  mp_clear (&t);
  mp_clear (&v);
#else
  *r += val;
#endif
}

static inline void
dinum_sub_u (dinum_t *r, uint64_t val)
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
//  mp_int    t;

  mp_init_u64 (&v, val);
//  mp_init (&t);
//  mp_copy (r, &t);
  mp_sub (r, &v, r);
//  mp_clear (&t);
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
//  mp_int    t;

//  mp_init (&t);
//  mp_copy (r, &t);
  mp_add (r, val, r);
//  mp_clear (&t);
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
//  mp_int    t;

//  mp_init (&t);
//  mp_copy (r, &t);
  mp_sub (r, val, r);
//  mp_clear (&t);
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
dinum_cmp_s (const dinum_t *r, int64_t val)
{
#if _use_math == DI_GMP
  return mpz_cmp_si (*r, val);
#elif _use_math == DI_TOMMATH
  mp_int      t;

  mp_init (&t);
  mp_set_i64 (&t, val);
  return mp_cmp (r, &t);
#else
  int64_t   t;
  int       rc = 0;

  t = *r;
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
//  mp_int    t;

//  mp_init (&t);
//  mp_copy (&t, r);
  mp_mul (r, (mp_int *) val, r);
//  mp_clear (&t);
#else
  *r *= *val;
#endif
}

static inline void
dinum_mul_u (dinum_t *r, uint64_t val)
{
#if _use_math == DI_GMP
  mpz_mul_ui (*r, *r, val);
#elif _use_math == DI_TOMMATH
//  mp_int    t;
  mp_int    v;

//  mp_init (&t);
//  mp_copy (&t, r);
  mp_init (&v);
  mp_set_u64 (&v, val);
  mp_mul (r, &v, r);
//  mp_clear (&t);
  mp_clear (&v);
#else
  *r *= val;
#endif
}

static inline void
dinum_mul_uu (dinum_t *r, uint64_t vala, uint64_t valb)
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
dinum_str (const dinum_t *r, char *str, size_t sz)
{
#if _use_math == DI_GMP
  gmp_snprintf (str, sz, "%Zd", *r);
#elif _use_math == DI_TOMMATH
  mp_to_decimal (r, str, sz);
#else
# if _hdr_inttypes
  snprintf (str, sz, "%" PRIu64, *r);
# elif _siz_long == 8
  snprintf (str, sz, "%ld", *r);
# elif _siz_long_long == 8
  snprintf (str, sz, "%lld", *r);
# endif
#endif
}

#endif /* DI_INC_DIMATH_H */
