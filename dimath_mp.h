/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#ifndef INC_DIMATH_MP_H
#define INC_DIMATH_MP_H

#include "config.h"

#if _use_math == DI_GMP
# include "dimath_gmp.h"
#elif _use_math == DI_MPDECIMAL
# include "dimath_mpdec.h"
#elif _use_math == DI_TOMMATH
# include "dimath_tommath.h"
#else
# include "dimath_internal.h"
#endif

#endif /* INC_DIMATH_MP_H */
