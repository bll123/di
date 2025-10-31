/* Copyright 2025 Brad Lanam Pleasant Hill CA */

/* This file exists to hold any extra variables that the MP */
/* routines may need */

#include "config.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _use_math == DI_MPDECIMAL
# include <mpdecimal.h>
#endif

#include "dimath.h"

#if _use_math == DI_MPDECIMAL
mpd_context_t  mpdctx;
#endif

int   dimathinitialized = 0;

void
dimath_initialize (void)
{
  if (dimathinitialized == 0) {
#if _use_math == DI_MPDECIMAL
    mpd_maxcontext (&mpdctx);
#endif
    dimathinitialized = 1;
  }
}

void
dimath_cleanup (void)
{
  return;
}
