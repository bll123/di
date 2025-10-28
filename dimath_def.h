/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#ifndef INC_DIMATH_DEF_H
#define INC_DIMATH_DEF_H

#if _hdr_stdint
# include <stdint.h>
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

#endif /* INC_DIMATH_DEF_H */
