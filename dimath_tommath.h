/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#ifndef INC_DIMATH_TOMMATH_H
#define INC_DIMATH_TOMMATH_H

#include "config.h"

#include "dimath_def.h"

// # pragma clang diagnostic push
// # pragma clang diagnostic ignored "-Wbad-function-cast"

#define MP_WUR
#if _hdr_tommath
# include <tommath.h>
#endif
#if _hdr_libtommath_tommath
# include <libtommath/tommath.h>
#endif

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

typedef mp_int dinum_t;

static inline void
dinum_init (dinum_t *r)
{
  mp_init_u64 (r, 0);
}

static inline void
dinum_clear (dinum_t *r)
{
  mp_clear (r);
}

static inline void
dinum_str (const dinum_t *r, char *str, Size_t sz)
{
  mp_to_decimal (r, str, sz);
}

static inline void
dinum_set (dinum_t *r, const dinum_t *val)
{
  mp_copy (val, r);
}

static inline void
dinum_set_u (dinum_t *r, di_ui_t val)
{
  mp_set_u64 (r, val);
}

static inline void
dinum_set_s (dinum_t *r, di_si_t val)
{
  mp_set_i64 (r, val);
}

static inline void
dinum_add_u (dinum_t *r, di_ui_t val)
{
  mp_int    v;

  mp_init_u64 (&v, val);
  mp_add (r, &v, r);
  mp_clear (&v);
}

static inline void
dinum_sub_u (dinum_t *r, di_ui_t val)
{
  mp_int    v;

  mp_init_u64 (&v, val);
  mp_sub (r, &v, r);
  mp_clear (&v);
}

static inline void
dinum_add (dinum_t *r, const dinum_t *val)
{
  mp_add (r, val, r);
}

static inline void
dinum_sub (dinum_t *r, const dinum_t *val)
{
  mp_sub (r, val, r);
}

static inline int
dinum_cmp (const dinum_t *r, const dinum_t *val)
{
  return mp_cmp (r, val);
}

static inline int
dinum_cmp_s (const dinum_t *r, di_si_t val)
{
  mp_int      t;
  int         rv;

  mp_init (&t);
  mp_set_i64 (&t, val);
  rv = mp_cmp (r, &t);
  mp_clear (&t);
  return rv;
}

static inline void
dinum_mul (dinum_t *r, const dinum_t *val)
{
  mp_mul (r, (mp_int *) val, r);
}

static inline void
dinum_mul_u (dinum_t *r, di_ui_t val)
{
  mp_int    v;

  mp_init (&v);
  mp_set_u64 (&v, val);
  mp_mul (r, &v, r);
  mp_clear (&v);
}

static inline void
dinum_mul_uu (dinum_t *r, di_ui_t vala, di_ui_t valb)
{
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
}

static inline double
dinum_scale (dinum_t *r, dinum_t *val)
{
  double    dval;

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

  return dval;
}

static inline double
dinum_perc (dinum_t *r, dinum_t *val)
{
  double      dval = 0.0;

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

  return dval;
}

static inline void
dimath_initialize (void)
{
  return;
}

static inline void
dimath_cleanup (void)
{
  return;
}

// # pragma clang diagnostic pop

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DIMATH_TOMMATH_H */
