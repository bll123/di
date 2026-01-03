/* Copyright 2025-2026 Brad Lanam Pleasant Hill CA */

#ifndef INC_DIMATH_GMP_H
#define INC_DIMATH_GMP_H

#include "config.h"

#if _hdr_stdio
# include <stdio.h>
#endif

#include "dimath.h"

// # pragma clang diagnostic push
// # pragma clang diagnostic ignored "-Wbad-function-cast"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wreserved-identifier"
#pragma gcc diagnostic push
#pragma gcc diagnostic ignored "-Wsign-conversion"
#include <gmp.h>
#pragma clang diagnostic pop
#pragma gcc diagnostic pop

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

typedef mpz_t dinum_t;

static inline void
dinum_init (dinum_t *r)
{
  mpz_init_set_ui (*r, (unsigned long) 0);
}

static inline void
dinum_clear (dinum_t *r)
{
  mpz_clear (*r);
}

static inline void
dinum_str (const dinum_t *r, char *str, Size_t sz)
{
  gmp_snprintf (str, sz, "%Zd", *r);
}

static inline void
dinum_set (dinum_t *r, const dinum_t *val)
{
  mpz_set (*r, *val);
}

static inline void
dinum_set_u (dinum_t *r, di_ui_t val)
{
  mpz_set_ui (*r, (unsigned long) val);
}

static inline void
dinum_set_s (dinum_t *r, di_si_t val)
{
  mpz_set_si (*r, (long) val);
}

static inline void
dinum_add_u (dinum_t *r, di_ui_t val)
{
  mpz_t     v;
  mpz_t     t;

  mpz_init_set_ui (v, (unsigned long) val);
  mpz_init (t);
  mpz_set (t, *r);
  mpz_add (*r, t, v);
  mpz_clear (t);
  mpz_clear (v);
}

static inline void
dinum_add (dinum_t *r, const dinum_t *val)
{
  mpz_t     t;

  mpz_init (t);
  mpz_set (t, *r);
  mpz_add (*r, t, *val);
  mpz_clear (t);
}

static inline void
dinum_sub_u (dinum_t *r, di_ui_t val)
{
  mpz_t     v;
  mpz_t     t;

  mpz_init_set_ui (v, (unsigned long) val);
  mpz_init (t);
  mpz_set (t, *r);
  mpz_sub (*r, t, v);
  mpz_clear (t);
  mpz_clear (v);
}

static inline void
dinum_sub (dinum_t *r, const dinum_t *val)
{
  mpz_t     t;

  mpz_init (t);
  mpz_set (t, *r);
  mpz_sub (*r, t, *val);
  mpz_clear (t);
}

static inline int
dinum_cmp (const dinum_t *r, const dinum_t *val)
{
  return mpz_cmp (*r, *val);
}

static inline int
dinum_cmp_s (const dinum_t *r, di_si_t val)
{
  return mpz_cmp_si (*r, (long) val);
}

static inline void
dinum_mul (dinum_t *r, const dinum_t *val)
{
  mpz_mul (*r, *r, *val);
}

static inline void
dinum_mul_u (dinum_t *r, di_ui_t val)
{
  mpz_mul_ui (*r, *r, (unsigned long) val);
}

static inline void
dinum_mul_uu (dinum_t *r, di_ui_t vala, di_ui_t valb)
{
  mpz_set_ui (*r, (unsigned long) 1);
  mpz_mul_ui (*r, *r, (unsigned long) vala);
  mpz_mul_ui (*r, *r, (unsigned long) valb);
}

static inline double
dinum_scale (dinum_t *r, dinum_t *val)
{
  double    dval;

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

  return dval;
}

static inline double
dinum_perc (dinum_t *r, dinum_t *val)
{
  double      dval = 0.0;

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

  return dval;
}

// # pragma clang diagnostic pop

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DIMATH_GMP_H */
