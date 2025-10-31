/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#ifndef INC_DIMATH_INTERNAL_H
#define INC_DIMATH_INTERNAL_H

#include "config.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stddef
# include <stddef.h>
#endif

#include "dimath.h"

// # pragma clang diagnostic push
// # pragma clang diagnostic ignored "-Wbad-function-cast"

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

typedef di_unum_t dinum_t;

static inline void
dinum_init (dinum_t *r)
{
  *r = 0;
}

static inline void
dinum_clear (dinum_t *r)
{
}

static inline void
dinum_str (const dinum_t *r, char *str, Size_t sz)
{
#if defined (DI_INTERNAL_DOUBLE)
# if _siz_long_double > 8
  Snprintf1 (str, sz, "%.0Lf", *r);
# else
  Snprintf1 (str, sz, "%.0f", *r);
# endif
#else
# if _hdr_inttypes && _siz_uint64_t == 8
  Snprintf1 (str, sz, "%" PRIu64, *r);
# elif _siz_long == 8
  Snprintf1 (str, sz, "%ld", *r);
# elif _siz_long_long == 8
  Snprintf1 (str, sz, "%lld", *r);
# else
  Snprintf1 (str, sz, "%d", *r);
# endif
#endif
}

static inline void
dinum_set (dinum_t *r, const dinum_t *val)
{
  *r = *val;
}

static inline void
dinum_set_u (dinum_t *r, di_ui_t val)
{
  *r = (dinum_t) val;
}

static inline void
dinum_set_s (dinum_t *r, di_si_t val)
{
  *r = (dinum_t) val;
}

static inline void
dinum_add_u (dinum_t *r, di_ui_t val)
{
  *r += (dinum_t) val;
}

static inline void
dinum_add (dinum_t *r, const dinum_t *val)
{
  *r += *val;
}

static inline void
dinum_sub_u (dinum_t *r, di_ui_t val)
{
  *r -= (dinum_t) val;
}

static inline void
dinum_sub (dinum_t *r, const dinum_t *val)
{
  *r -= *val;
}

static inline int
dinum_cmp (const dinum_t *r, const dinum_t *val)
{
  int     rc = 0;

  if (*r < *val) {
    rc = -1;
  } else if (*r > *val) {
    rc = 1;
  }
  return rc;
}

static inline int
dinum_cmp_s (const dinum_t *r, di_si_t val)
{
  di_snum_t   t;
  int         rc = 0;

  t = (di_snum_t) *r;
  if (t < val) {
    rc = -1;
  } else if (t > val) {
    rc = 1;
  }
  return rc;
}

static inline void
dinum_mul (dinum_t *r, const dinum_t *val)
{
  *r *= *val;
}

static inline void
dinum_mul_u (dinum_t *r, di_ui_t val)
{
  *r *= (dinum_t) val;
}

static inline void
dinum_mul_uu (dinum_t *r, di_ui_t vala, di_ui_t valb)
{
  *r = (dinum_t) vala;
  *r *= (dinum_t) valb;
}

static inline double
dinum_scale (dinum_t *r, dinum_t *val)
{
  double    dval;

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

  return dval;
}

static inline double
dinum_perc (dinum_t *r, dinum_t *val)
{
  double      dval = 0.0;

#if defined (DI_INTERNAL_DOUBLE)
  dval = (double) (*r / *val);
#endif
#if defined (DI_INTERNAL_INT)
  /* in the case of a uint, simply convert and divide */
  dval = (double) *r / (double) *val;
#endif
  dval *= 100.0;

  return dval;
}

// # pragma clang diagnostic pop

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DIMATH_INTERNAL_H */
